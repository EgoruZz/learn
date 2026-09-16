# AGENTS.md

## Структура

- **~/learn/** — рабочая папка проекта (ты здесь)
- **~/stack/** — инструменты: MCP серверы, Python venv, скрипты
- **~/projects/** — другие проекты

Запускай OpenCode отсюда: `cd ~/learn && opencode`

## Инструменты (из ~/stack/)

Все MCP серверы (Wolfram, Julia, Manim, ParaView, tldraw) настроены глобально и доступны из любой папки.
Python venv и скрипты вызываются по абсолютному пути:

### Wolfram Language
- **Wolfram Engine 15.0** — бесплатная лицензия для разработчиков
- Запуск: `wolframscript`
- MCP сервер: `Wolfram` (в конфиге ~/.config/opencode/opencode.jsonc)
- Jupyter kernel: `wolframlanguage15` (для Jupyter notebooks)
- Лицензия: продлевать на https://www.wolfram.com/engine/ каждые ~40 дней
- Проверка лицензии: `wolframscript -code 'Print[$LicenseExpirationDate]'`
- Пакеты: ~/stack/WolframLanguage/ (пользовательские пакеты, создаются по мере необходимости)
- Jupyter интеграция: ~/stack/WolframLanguageForJupyter/

### Manim
- **Manim 0.21.0** (community edition) — основная версия для анимаций
- **ManimGL 1.7.2** (3Blue1Brown) — OpenGL версия
- Окружение: ~/stack/manim-env (Python 3.12)
- Активация: `source ~/stack/manim-env/bin/activate` или `manim-activate`
- Запуск (community): `manim script.py SceneName`
- Запуск (GL): `manimgl script.py SceneName`
- MCP сервер: `manim` (в конфиге ~/.config/opencode/opencode.jsonc)
- MCP использует community `manim` для headless-рэндера; `manimgl` остаётся отдельным интерактивным CLI

### ParaView
- **ParaView 6.2.0-RC1** (GUI + pvpython)
- pvpython: `/Applications/ParaView-6.2.0-RC1.app/Contents/bin/pvpython`
- Python API: `paraview.simple`
- Скрипты: писать → запускать через pvpython или вставлять в GUI Python Shell
- MCP сервер: `ParaView` (в конфиге ~/.config/opencode/opencode.jsonc)
- Окружение: ~/stack/paraview-mcp-env (Python 3.12)
- MCP инструменты: load_data, create_source, create_isosurface, create_slice, toggle_volume_rendering, color_by, rotate_camera, reset_camera, plot_over_line, warp_by_vector, create_streamline, get_pipeline, и др.
- **Скриншоты:** папка `~/stack/paraview-mcp/screenshots/`
  - Сохраняй скриншоты туда с понятными именами (например `isosurface_temp.png`)
  - Говори агенту: "скриншот в ~/stack/paraview-mcp/screenshots/имя.png — можешь смотреть"
  - MCP скриншоты не доступны (bridge работает в отдельном pvprocess и не видит render view GUI)
- **Workflow запуска (обязательно!):**
  1. Запустить ParaView GUI: `open /Applications/ParaView-6.2.0-RC1.app`
  2. Запустить pvserver: `/Applications/ParaView-6.2.0-RC1.app/Contents/bin/pvserver --multi-clients --server-port=11111`
  3. В GUI: File → Connect → localhost:11111
  4. Запустить TCP-мост: `/Applications/ParaView-6.2.0-RC1.app/Contents/bin/pvpython ~/stack/paraview-mcp/paraview_bridge.py`
  5. MCP сервер подключается к мосту автоматически
- **Workflow завершения:**
  1. Закрыть ParaView GUI (просто закрыть окно)
  2. Остановить pvserver: `kill $(pgrep -f pvserver)`
  3. Остановить bridge: `kill $(pgrep -f paraview_bridge)`

### LaTeX
- **MacTeX 2026** — `pdflatex` в PATH
- Запуск: `pdflatex document.tex`
- Интеграция: расширение LaTeX Workshop в VS Code

### Julia
- **Julia 1.12.7** (через juliaup)
- Запуск: `julia`
- MCP сервер: `julia` (в конфиге ~/.config/opencode/opencode.jsonc)
- Пакеты: LinearAlgebra, SparseArrays, Plots, DifferentialEquations, StaticArrays, Optim, JuMP, IJulia, Flux, Turing, DataFrames, CSV, ForwardDiff, SymPy, SpecialFunctions, StatsBase, Distributions, Polynomials, FFTW, IterativeSolvers

### tldraw
- **tldraw MCP** — управление canvas через MCP (30 инструментов)
- MCP сервер: `tldraw` (в конфиге ~/.config/opencode/opencode.jsonc)
- Сервер: ~/stack/tldraw-mcp/ (Node.js, zero browser deps)
- Скилл: `tldraw-skill` — генерация .tldr диаграмм из текста (6 presets, vision self-check)
- Инструменты: create_geo, create_text, create_arrow, create_note, create_frame, move_shapes, group_shapes, set_camera, save_document, load_document и др.
- Экспорт: `.tldr` → PNG/SVG через `@kitschpatrol/tldraw-cli`; после обновления CLI нужно проверять совместимость схемы MCP-файлов.
- Текущий MCP надёжно создаёт/сохраняет `.tldr` и редактирует документ; live-синхронизация с уже открытым desktop canvas не гарантируется.

### Jupyter
- **JupyterLab** окружение: ~/stack/jupyter-env
- Запуск: `source ~/stack/jupyter-env/bin/activate && jupyter-lab`
- Или: `jupyter-jlab`
- Kernel: Python 3.12 + Wolfram Language 15
- Пакеты: numpy, matplotlib, pandas, sympy, plotly, ipywidgets
- XLSX support: openpyxl, markitdown

### VS Code
- Расширения: Jupyter, Python, Julia, LaTeX Workshop, Wolfram Language
- Конфиг: ~/Library/Application Support/Code/User/settings.json

### OpenCode
- Конфиг MCP: ~/.config/opencode/opencode.jsonc
- Skills: ARIS research pipeline, Firecrawl, render-visual, xlsx, data-analysis, grill-me, tldraw-skill, epistemic-rigor, deep-mind, dag-hallucination-detector, и др. (50+ скиллов, полный список в ~/.config/opencode/skills/)
- ARIS работает в local-only режиме: внешние Codex/Oracle/manual-review MCP не используются и не считаются доступными.
- Для многораундовой проверки используются локальные OpenCode subagents; результат нельзя называть независимой cross-model экспертизой.
- Для исследовательских утверждений явно разделять verified, inferred и unknown; отсутствие источника не считать подтверждением.

### Визуализация и маршрутизация
- `render-visual` — polished PNG/SVG/PDF, слайды, архитектурные схемы и GIF через headless Chromium.
- `tldraw-skill` + tldraw MCP — редактируемые whiteboard `.tldr` документы и shapes.
- Если нужен data chart из числового набора, использовать `data-analysis`/matplotlib/plotly, а не schematic `render-visual` charts.
- Если запрос неоднозначен между tldraw и render-visual, сначала уточнить нужный результат: редактируемый canvas или финальная картинка.

## Алиасы (в .zshrc)
- `manim-activate` — активировать окружение manim-env
- `jupyter-jlab` — запустить JupyterLab
- `jupyter-wolfram` — запустить Jupyter с Wolfram kernel
- `stack` — перейти в ~/stack/

## Полная документация
См. ~/stack/STACK.md — горячие клавиши, ссылки, обновление, визуализация.
- прочитать обязательно

## Конвенции

- Все Python проекты в venv (PEP 668 защищает системный Python)
- Julia — через juliaup, пакеты в глобальном окружении
- Wolfram — wolframscript для CLI, MCP для AI интеграции
- Скрипты для ParaView писать на Python (`paraview.simple`)
- Запускай OpenCode из ~/learn/ чтобы я находил AGENTS.md

## Управление пакетами

### Manim (~/stack/manim-env)
```bash
source ~/stack/manim-env/bin/activate
pip list                          # список пакетов
pip install <package>             # добавить
pip install --upgrade <package>   # обновить
pip install --upgrade manim manimgl  # обновить всё
```

### Jupyter (~/stack/jupyter-env)
```bash
source ~/stack/jupyter-env/bin/activate
pip list
pip install <package>
pip install --upgrade jupyterlab
```

### Julia
```julia
julia
using Pkg
Pkg.status()          # список пакетов
Pkg.add("PackageName") # добавить
Pkg.update()          # обновить всё
```

### Wolfram
```bash
wolframscript -code 'PacletInformation[]'  # список пакетов
wolframscript -code 'PacletInstall["PackageName"]'  # добавить
wolframscript -code 'PacletUpdate[]'  # обновить
```

### Проверка полноты
```bash
cd ~/stack && bash check-all.sh  # проверить что всё работает
```
