/* ============================================
   KNOWLEDGE BASE — NAVIGATION
   ============================================ */

(function () {
    'use strict';

    // --- Section metadata ---
    var SECTIONS = {
        'math': { name: 'Математика', subsections: {
            'discrete-and-logic': { name: 'Дискретная математика и логика', letters: 'a,b,c,d'.split(',') },
            'number-theory': { name: 'Теория чисел', letters: 'a,b,c,d,e'.split(',') },
            'combinatorics': { name: 'Комбинаторика', letters: 'a,b,c,d'.split(',') },
            'algebra': { name: 'Алгебра', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
            'linear-algebra': { name: 'Линейная алгебра', letters: 'a,b,c,d,e,f,g,h,i,j,k,l,m'.split(',') },
            'analysis': { name: 'Матанализ', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
            'prob-and-stats': { name: 'Теория вероятностей', letters: 'a,b,c,d,e,f,g,h,i'.split(',') },
            'pictures': { name: 'Графика', letters: 'a,b'.split(',') },
            'geometry': { name: 'Геометрия', letters: 'a,b,c,d,e,f,g,h'.split(',') }
        }},
        'struct': { name: 'Структуры данных', letters: 'a,b,c,d,e,f,g,h,i,j,k'.split(',') },
        'dynamic': { name: 'Динамическое программирование', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
        'graph': { name: 'Теория графов', letters: 'a,b,c,d,e,f,g,h,i,j,k,l'.split(',') },
        'string': { name: 'Строки', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
        'technique': { name: 'Техники и алгоритмы', letters: 'a,b,c,cpp,d,e,f,g'.split(',') },
        'hashing': { name: 'Хеширование', letters: 'a,b,c,d,e,f,g,h,i,j'.split(',') },
        'geometry': { name: 'Геометрия', letters: [] },
        'algo-analysis': { name: 'Анализ алгоритмов', letters: 'a,b,c,d,e,f,g,h'.split(',') },
        'special-numbers': { name: 'Специальные числа', letters: [] },
        'special-sequences': { name: 'Специальные последовательности', letters: [] }
    };

    var ROOT_NAME = 'Главная';

    // --- Parse current URL ---
    function parseURL() {
        var path = window.location.pathname;
        // Remove leading/trailing slashes and split
        var parts = path.replace(/^\/+|\/+$/g, '').split('/');
        // Remove 'learn-production' if present
        if (parts[0] === 'learn-production') parts.shift();
        return parts;
    }

    // --- Build breadcrumb items ---
    function buildBreadcrumbs(parts) {
        var items = [];
        var depth = parts.length;
        var href = '';

        // Root
        href = depth > 0 ? '../'.repeat(depth) : './';
        items.push({ name: ROOT_NAME, href: href, isCurrent: depth === 0 });

        if (depth === 0) return items;

        var section = parts[0];
        var sectionMeta = SECTIONS[section];
        if (!sectionMeta) return items;

        // Section
        href = depth > 1 ? '../'.repeat(depth - 1) : './';
        items.push({ name: sectionMeta.name, href: href, isCurrent: depth === 1 });

        if (depth === 1) return items;

        var subsection = parts[1];
        var subMeta = sectionMeta.subsections ? sectionMeta.subsections[subsection] : null;

        // Subsection (math only)
        if (subMeta) {
            href = depth > 2 ? '../'.repeat(depth - 2) : './';
            items.push({ name: subMeta.name, href: href, isCurrent: depth === 2 });

            if (depth === 2) return items;

            // Letter in subsection
            var letter = parts[2];
            items.push({ name: letter.toUpperCase(), href: './', isCurrent: depth === 3 });
        } else {
            // Letter directly in section
            var letter = parts[1];
            items.push({ name: letter.toUpperCase(), href: './', isCurrent: depth === 2 });
        }

        return items;
    }

    // --- Render breadcrumbs ---
    function renderBreadcrumbs() {
        var el = document.getElementById('breadcrumbs');
        if (!el) return;

        var parts = parseURL();
        var items = buildBreadcrumbs(parts);

        var html = '';
        for (var i = 0; i < items.length; i++) {
            if (i > 0) {
                html += '<span class="separator">›</span>';
            }
            if (items[i].isCurrent) {
                html += '<span class="current">' + items[i].name + '</span>';
            } else {
                html += '<a href="' + items[i].href + '">' + items[i].name + '</a>';
            }
        }
        el.innerHTML = html;
    }

    // --- Render sidebar ---
    function renderSidebar() {
        var parts = parseURL();
        if (parts.length === 0) return;

        var section = parts[0];
        var sectionMeta = SECTIONS[section];
        if (!sectionMeta) return;

        // Determine if we're in a subsection (math) or directly in section
        var subsection = null;
        var letters = sectionMeta.letters || [];
        var basePath = '../';

        if (sectionMeta.subsections && parts.length > 1 && parts[1] !== 'index.html') {
            subsection = parts[1];
            var subMeta = sectionMeta.subsections[subsection];
            if (subMeta) {
                letters = subMeta.letters;
                basePath = '../../';
            }
        }

        // Build sidebar HTML
        var html = '<nav class="sidebar">';
        html += '<div class="sidebar-title">' + sectionMeta.name + '</div>';
        html += '<ul class="sidebar-list">';

        if (subsection) {
            // Show all subsections
            var subKeys = Object.keys(sectionMeta.subsections);
            for (var i = 0; i < subKeys.length; i++) {
                var key = subKeys[i];
                var meta = sectionMeta.subsections[key];
                var active = key === subsection ? ' active' : '';
                html += '<li class="sidebar-subsection' + active + '">';
                html += '<a href="' + basePath + key + '/index.html">' + meta.name + '</a>';
                html += '</li>';
            }
        } else if (letters.length > 0) {
            // Show letters
            for (var i = 0; i < letters.length; i++) {
                var l = letters[i];
                var currentLetter = parts.length > 1 ? parts[1] : null;
                // Handle 'cpp' in technique
                var displayName = l === 'cpp' ? 'C++' : l.toUpperCase();
                var active = l === currentLetter ? ' active' : '';
                html += '<li class="sidebar-letter' + active + '">';
                html += '<a href="' + basePath + l + '/' + l + '.html">' + displayName + '</a>';
                html += '</li>';
            }
        }

        html += '</ul></nav>';

        // Insert sidebar before main content or at body start
        var main = document.querySelector('main.content');
        if (main) {
            main.insertAdjacentHTML('beforebegin', html);
        }
    }

    // --- Add sidebar CSS dynamically ---
    function addSidebarStyles() {
        var style = document.createElement('style');
        style.textContent = [
            '.sidebar {',
            '  position: fixed;',
            '  top: 0;',
            '  left: 0;',
            '  width: var(--sidebar-width);',
            '  height: 100vh;',
            '  background: var(--bg-sidebar);',
            '  border-right: 1px solid var(--border);',
            '  overflow-y: auto;',
            '  padding: 1.5rem 0;',
            '  z-index: 100;',
            '}',
            '.sidebar-title {',
            '  padding: 0 1.25rem;',
            '  font-size: 0.8rem;',
            '  font-weight: 600;',
            '  text-transform: uppercase;',
            '  letter-spacing: 0.05em;',
            '  color: var(--text-muted);',
            '  margin-bottom: 0.75rem;',
            '}',
            '.sidebar-list {',
            '  list-style: none;',
            '  padding: 0;',
            '  margin: 0;',
            '}',
            '.sidebar-list li {',
            '  margin: 0;',
            '}',
            '.sidebar-list a {',
            '  display: block;',
            '  padding: 0.35rem 1.25rem;',
            '  font-size: 0.88rem;',
            '  color: var(--text);',
            '  text-decoration: none;',
            '  transition: background 0.15s, color 0.15s;',
            '}',
            '.sidebar-list a:hover {',
            '  background: var(--border-light);',
            '  color: var(--accent);',
            '  text-decoration: none;',
            '}',
            '.sidebar-list .active a {',
            '  color: var(--accent);',
            '  font-weight: 600;',
            '  background: var(--accent-light);',
            '}',
            '.sidebar-subsection {',
            '  border-bottom: 1px solid var(--border-light);',
            '  padding-bottom: 0.25rem;',
            '  margin-bottom: 0.25rem;',
            '}',
            '',
            '/* Offset main content for sidebar */',
            '.site-header, .content {',
            '  margin-left: var(--sidebar-width);',
            '}',
            '',
            '/* Responsive: hide sidebar on mobile */',
            '@media (max-width: 900px) {',
            '  .sidebar {',
            '    position: static;',
            '    width: 100%;',
            '    height: auto;',
            '    border-right: none;',
            '    border-bottom: 1px solid var(--border);',
            '    padding: 0.75rem 0;',
            '  }',
            '  .sidebar-list {',
            '    display: flex;',
            '    flex-wrap: wrap;',
            '    padding: 0 0.75rem;',
            '    gap: 0.25rem;',
            '  }',
            '  .sidebar-list li {',
            '    flex: 0 0 auto;',
            '  }',
            '  .sidebar-list a {',
            '    padding: 0.25rem 0.6rem;',
            '    border-radius: var(--radius);',
            '    font-size: 0.82rem;',
            '  }',
            '  .sidebar-title {',
            '    display: none;',
            '  }',
            '  .sidebar-subsection {',
            '    border: none;',
            '    padding: 0;',
            '    margin: 0;',
            '  }',
            '  .site-header, .content {',
            '    margin-left: 0;',
            '  }',
            '}'
        ].join('\n');
        document.head.appendChild(style);
    }

    // --- Init ---
    function init() {
        addSidebarStyles();
        renderBreadcrumbs();
        renderSidebar();
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
