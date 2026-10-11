/* ============================================
   KNOWLEDGE BASE — NAVIGATION (full tree)
   ============================================ */

(function () {
    'use strict';

    // --- Full tree structure ---
    var TREE = [
        { id: 'math', name: 'Математика', children: [
            { id: 'discrete-and-logic', name: 'Дискретная математика и логика', letters: 'a,b,c,d'.split(',') },
            { id: 'number-theory', name: 'Теория чисел', letters: 'a,b,c,d,e'.split(',') },
            { id: 'combinatorics', name: 'Комбинаторика', letters: 'a,b,c,d'.split(',') },
            { id: 'algebra', name: 'Алгебра', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
            { id: 'linear-algebra', name: 'Линейная алгебра', letters: 'a,b,c,d,e,f,g,h,i,j,k,l,m'.split(',') },
            { id: 'analysis', name: 'Матанализ', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
            { id: 'prob-and-stats', name: 'Теория вероятностей', letters: 'a,b,c,d,e,f,g,h,i'.split(',') },
            { id: 'pictures', name: 'Графика', letters: 'a,b'.split(',') },
            { id: 'geometry', name: 'Геометрия', letters: 'a,b,c,d,e,f,g,h'.split(',') }
        ]},
        { id: 'struct', name: 'Структуры данных', letters: 'a,b,c,d,e,f,g,h,i,j,k'.split(',') },
        { id: 'dynamic', name: 'Динамическое программирование', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
        { id: 'graph', name: 'Теория графов', letters: 'a,b,c,d,e,f,g,h,i,j,k,l'.split(',') },
        { id: 'string', name: 'Строки', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
        { id: 'technique', name: 'Техники и алгоритмы', letters: 'a,b,c,cpp,d,e,f,g'.split(',') },
        { id: 'hashing', name: 'Хеширование', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
        { id: 'algo-analysis', name: 'Анализ алгоритмов', letters: 'a,b,c,d,e,f,g,h'.split(',') },
        { id: 'geometry', name: 'Геометрия', letters: [] },
        { id: 'special-numbers', name: 'Специальные числа', letters: [] },
        { id: 'special-sequences', name: 'Специальные последовательности', letters: [] }
    ];

    // --- Parse URL ---
    function parseURL() {
        var path = window.location.pathname;
        var parts = path.replace(/^\/+|\/+$/g, '').split('/');
        if (parts[0] === 'learn-production') parts.shift();
        if (parts[parts.length - 1] === 'index.html') parts.pop();
        return parts;
    }

    // --- Build relative path from current page ---
    function relPath(depth) {
        return '../'.repeat(depth);
    }

    // --- Render full tree sidebar ---
    function renderSidebar() {
        var parts = parseURL();
        var depth = parts.length;
        var rel = relPath(depth);

        var html = '<nav class="sidebar">';
        html += '<div class="sidebar-title">Навигация</div>';
        html += '<ul class="sidebar-list">';

        // Link to root
        html += '<li><a href="' + rel + 'index.html"' +
            (depth === 0 ? ' class="active"' : '') +
            '>Главная</a></li>';

        for (var s = 0; s < TREE.length; s++) {
            var section = TREE[s];
            var isCurrentSection = parts[0] === section.id;
            var sectionOpen = isCurrentSection || depth === 0;
            var sectionRel = depth > 0 ? rel : '';

            html += '<li class="sidebar-section">';
            html += '<a href="' + sectionRel + section.id + '/index.html"' +
                (isCurrentSection && depth <= 1 ? ' class="active"' : '') +
                '>' + section.name + '</a>';

            // Show children if this section is current or if at root
            if (section.children && sectionOpen) {
                html += '<ul class="sidebar-sub">';

                for (var c = 0; c < section.children.length; c++) {
                    var sub = section.children[c];
                    var isCurrentSub = isCurrentSection && parts[1] === sub.id;
                    var subRel = depth > 1 ? rel : (isCurrentSection ? rel : '');

                    html += '<li class="sidebar-subsection">';
                    html += '<a href="' + subRel + section.id + '/' + sub.id + '/index.html"' +
                        (isCurrentSub && depth <= 2 ? ' class="active"' : '') +
                        '>' + sub.name + '</a>';

                    // Show letters if this subsection is current
                    if (isCurrentSub && sub.letters && sub.letters.length > 0) {
                        html += '<ul class="sidebar-letters">';
                        for (var l = 0; l < sub.letters.length; l++) {
                            var letter = sub.letters[l];
                            var displayName = letter === 'cpp' ? 'C++' : letter.toUpperCase();
                            var isCurrentLetter = parts[2] === letter;
                            var letterRel = depth > 2 ? rel : '';
                            html += '<li><a href="' + letterRel + section.id + '/' + sub.id + '/' + letter + '/' + letter + '.html"' +
                                (isCurrentLetter ? ' class="active"' : '') +
                                '>' + displayName + '</a></li>';
                        }
                        html += '</ul>';
                    }

                    html += '</li>';
                }

                html += '</ul>';
            } else if (!section.children && section.letters && section.letters.length > 0 && sectionOpen) {
                // Direct letters (struct, dynamic, etc.)
                html += '<ul class="sidebar-letters">';
                for (var l = 0; l < section.letters.length; l++) {
                    var letter = section.letters[l];
                    var displayName = letter === 'cpp' ? 'C++' : letter.toUpperCase();
                    var isCurrentLetter = isCurrentSection && parts[1] === letter;
                    var letterRel = depth > 1 ? rel : '';
                    html += '<li><a href="' + letterRel + section.id + '/' + letter + '/' + letter + '.html"' +
                        (isCurrentLetter ? ' class="active"' : '') +
                        '>' + displayName + '</a></li>';
                }
                html += '</ul>';
            }

            html += '</li>';
        }

        html += '</ul></nav>';

        var main = document.querySelector('main.content');
        if (main) {
            main.insertAdjacentHTML('beforebegin', html);
        }
    }

    // --- Breadcrumbs ---
    function renderBreadcrumbs() {
        var el = document.getElementById('breadcrumbs');
        if (!el) return;

        var parts = parseURL();
        var depth = parts.length;
        var items = [];

        // Root
        items.push({ name: 'Главная', href: relPath(depth) + 'index.html', isCurrent: depth === 0 });
        if (depth === 0) { el.innerHTML = '<span class="current">Главная</span>'; return; }

        // Find section
        var sectionId = parts[0];
        var sectionName = sectionId;
        var subName = null;
        var letterName = null;

        for (var s = 0; s < TREE.length; s++) {
            if (TREE[s].id === sectionId) {
                sectionName = TREE[s].name;
                if (TREE[s].children && parts[1]) {
                    for (var c = 0; c < TREE[s].children.length; c++) {
                        if (TREE[s].children[c].id === parts[1]) {
                            subName = TREE[s].children[c].name;
                            if (parts[2]) letterName = parts[2].toUpperCase();
                        }
                    }
                } else if (TREE[s].letters && parts[1]) {
                    letterName = parts[1].toUpperCase();
                }
                break;
            }
        }

        items.push({ name: sectionName, href: relPath(depth - 1) + sectionId + '/index.html', isCurrent: depth === 1 });

        if (subName) {
            items.push({ name: subName, href: relPath(Math.max(depth - 2, 0)) + sectionId + '/' + parts[1] + '/index.html', isCurrent: depth === 2 });
        }

        if (letterName && depth > 2) {
            items.push({ name: letterName, href: '#', isCurrent: true });
        } else if (letterName && !subName && depth > 1) {
            items.push({ name: letterName, href: '#', isCurrent: true });
        }

        var html = '';
        for (var i = 0; i < items.length; i++) {
            if (i > 0) html += '<span class="separator">›</span>';
            if (items[i].isCurrent) {
                html += '<span class="current">' + items[i].name + '</span>';
            } else {
                html += '<a href="' + items[i].href + '">' + items[i].name + '</a>';
            }
        }
        el.innerHTML = html;
    }

    // --- CSS ---
    function addStyles() {
        var style = document.createElement('style');
        style.textContent = `
            .sidebar {
                position: fixed; top: 0; left: 0;
                width: var(--sidebar-width); height: 100vh;
                background: var(--bg-sidebar);
                border-right: 1px solid var(--border);
                overflow-y: auto; padding: 1.5rem 0; z-index: 100;
            }
            .sidebar-title {
                padding: 0 1.25rem; font-size: 0.8rem; font-weight: 600;
                text-transform: uppercase; letter-spacing: 0.05em;
                color: var(--text-muted); margin-bottom: 0.75rem;
            }
            .sidebar-list { list-style: none; padding: 0; margin: 0; }
            .sidebar-list li { margin: 0; }
            .sidebar-list a {
                display: block; padding: 0.3rem 1.25rem;
                font-size: 0.85rem; color: var(--text);
                text-decoration: none; transition: background 0.15s, color 0.15s;
            }
            .sidebar-list a:hover { background: var(--border-light); color: var(--accent); text-decoration: none; }
            .sidebar-list a.active { color: var(--accent); font-weight: 600; background: var(--accent-light); }
            .sidebar-section > a { font-weight: 600; font-size: 0.9rem; padding-top: 0.5rem; }
            .sidebar-sub { list-style: none; padding: 0; }
            .sidebar-sub > li > a { padding-left: 1.75rem; font-size: 0.83rem; }
            .sidebar-letters { list-style: none; padding: 0; }
            .sidebar-letters li a { padding-left: 2.5rem; font-size: 0.8rem; color: var(--text-muted); }
            .sidebar-letters li a.active { color: var(--accent); }
            .site-header, .content { margin-left: var(--sidebar-width); }
            @media (max-width: 900px) {
                .sidebar { position: static; width: 100%; height: auto; border-right: none; border-bottom: 1px solid var(--border); padding: 0.75rem 0; }
                .sidebar-list { display: flex; flex-wrap: wrap; padding: 0 0.75rem; gap: 0.25rem; }
                .sidebar-list li { flex: 0 0 auto; }
                .sidebar-list a { padding: 0.25rem 0.6rem; border-radius: var(--radius); font-size: 0.82rem; }
                .sidebar-title, .sidebar-sub, .sidebar-letters, .sidebar-section > a:first-child + .sidebar-sub { display: none; }
                .site-header, .content { margin-left: 0; }
            }
        `;
        document.head.appendChild(style);
    }

    // --- Init ---
    function init() {
        addStyles();
        renderBreadcrumbs();
        renderSidebar();
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
