---
name: tldraw
description: Управляет холстом tldraw offline (десктоп-приложение) через локальный HTTP API: читает открытые документы и фигуры, создаёт боксы/стрелки/текст/стикеры, делает скриншоты. Использовать, когда нужно нарисовать или отредактировать диаграмму, схему алгоритма, mind map, заметки на whiteboard в tldraw offline. Триггеры: tldraw, whiteboard, нарисуй схему, диаграмма, canvas.
---

# tldraw offline canvas

tldraw offline — десктоп-приложение (`offline.tldraw.com`), файлы `.tldraw`.
Приложение должно быть **запущено и держать нужный документ открытым**. Управляем
через локальный HTTP API.

## Доступ

- База: `http://localhost:7236`
- Токен читаем из `~/Library/Application Support/tldraw/server.json` (macOS),
  Linux: `~/.config/tldraw/server.json`, Windows (Git Bash): `$APPDATA/tldraw/server.json`.
  Токен меняется при каждом запуске приложения — **читать его заново в каждой команде**.

```bash
TOKEN=$(python3 -c "import json;print(json.load(open('$HOME/Library/Application Support/tldraw/server.json'))['token'])")
curl -s -X POST http://localhost:7236/api/search \
  -H 'content-type: application/json' \
  -H "authorization: Bearer $TOKEN" \
  -d '{"code":"..."}'
```

## Эндпоинты

| Что | Команда |
|---|---|
| Читать открытые документы / фигуры / скриншоты | `POST /api/search` с кодом на `api.*` |
| Создать новый документ | `POST /api/docs/create` `{"name":"Имя","directory":"/путь"}` |
| Мутировать редактор | `POST /api/doc/:id/exec` с кодом на `editor` + `helpers` |
| Постоянные скрипты (своё поведение файла) | `POST /api/doc/:id/script-workspace` → править `mainJsPath` |
| Статус скрипта | `GET /api/doc/:id/script-status` |

Полная документация: `GET http://localhost:7236/readme` с токеном.

## Стандартный цикл

1. Найти документ: `{"code":"return await api.getFocusedDoc()"}` — это обычно нужный
   документ (или фильтровать: `{"code":"return await api.getDocs({ name: \"canvas\" })"}`).
2. Прочитать текущие фигуры, чтобы понять координаты и ничего не затереть:
   `{"code":"const d=await api.getFocusedDoc(); const p=d?await api.getShapes(d.id):null; return {doc:d, shapes:p?.shapes.map(s=>({id:s.id,type:s.type,x:s.x,y:s.y}))??[]}"}`
3. Создать/обновить фигуры через `/api/doc/:id/exec`, завершить `await helpers.saveDoc()`.
4. Один раз проверить через `api.getShapes()` (скриншот — только если позиции спорны или пользователь просил).

## Создание фигур (примеры exec)

Примитивы SDK импортируются динамически: `const { createShapeId, toRichText } = await import('tldraw')`.

```js
// Бокс с текстом
const { createShapeId, toRichText } = await import('tldraw');
editor.createShape({
  id: createShapeId('box1'), type: 'geo', x: 100, y: 100,
  props: { geo: 'rectangle', w: 300, h: 200, richText: toRichText('Label') }
});

// Текст
editor.createShape({
  id: createShapeId('t1'), type: 'text', x: 240, y: 160,
  props: { richText: toRichText('Строка 1\nСтрока 2') }
});

// Стикер
editor.createShape({
  id: createShapeId('note1'), type: 'note', x: 200, y: 200,
  props: { color: 'yellow', richText: toRichText('важно') }
});

// Два бокса + стрелка между ними (bindings создаёт хелпер)
const b1 = createShapeId('b1'), b2 = createShapeId('b2');
editor.createShapes([
  { id: b1, type: 'geo', x: 120, y: 140, props: { geo: 'rectangle', w: 260, h: 140, richText: toRichText('A') } },
  { id: b2, type: 'geo', x: 520, y: 140, props: { geo: 'rectangle', w: 260, h: 140, richText: toRichText('B') } }
]);
const arrow = helpers.createArrowBetweenShapes(b1, b2, { richText: toRichText('связь') });
editor.select(b1, b2, arrow);
editor.zoomToFit({ animation: { duration: 200 } });
await helpers.saveDoc();
```

Мутирующий exec обязан заканчиваться `await helpers.saveDoc()`. Возвращать простой JSON
(ids, count, boolean). После создания проверить `api.getShapes()`.

## Ориентиры компоновки

- Текстовые боксы сами растут под контент.
- Стандартный размер фигуры с текстом ~300×200.
- Стандартный зазор между фигурами ~200.
- Смысловые связи — только через `helpers.createArrowBetweenShapes` (реальные bindings), не «похожие» стрелки.

## Полезные хелперы

- `helpers.getLints()` — линтер холста, запускать перед объявлением схемы готовой.
- `helpers.richTextToPlainText(richText)` — вытащить текст.
- `helpers.boxShapes` / `helpers.createShapeIfMissing` / `helpers.createShapesIfMissing` — идемпотентное создание.
- `helpers.onShapeTranslate` — подписка на перемещение фигур (для скриптов).
- `editor.zoomToFit({ animation: { duration: 200 } })` — навести камеру на всё.
- `editor.select(...ids)` — выделить.

## Постоянные document-скрипты

Если поведение должно переживать перезапуск (кнопки, горячие клавиши, анимация, «построить схему при открытии»):
`POST /api/doc/:id/script-workspace` → вернёт `mainJsPath` (папка `script/` в рабочей копии) →
редактируем файл → ждём пару секунд → проверяем `script-status` (`state: "applied"`).
В скрипте работает статический `import { ... } from 'tldraw'`. Не вызывать `helpers.saveDoc()` из скрипта.
Никогда не редактировать `.tldraw` архив и `db.sqlite` напрямую, пока файл открыт в приложении.

## Математика

В tldraw нет нативного LaTeX. Варианты: юникод/обычный текст (`n²`, `αᵢ`, `√n`, `Σ`),
или отрендерить формулу в PNG/SVG отдельно и вставить картинкой (`editor.createShape({ type: 'image', ... })`).

## Git

Файлы `.tldraw` можно хранить в репозитории. Перед git-операциями (pull/rebase) документ
нужно закрыть в приложении — tldraw offline не мёржит внешние изменения открытого файла.
