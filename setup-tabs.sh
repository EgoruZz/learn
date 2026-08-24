#!/bin/bash
# Настройка вкладок iTerm2 и окон VS Code для параллельной работы
# Ветки: learning1 (linear-algebra), learning2 (analysis)

DIR="$HOME/learn"

# --- iTerm2: 3 вкладки ---
osascript <<EOF
tell application "iTerm2"
    activate
    
    -- Вкладка 1: learning1 (linear-algebra)
    set t1 to (current tab of current window)
    tell t1
        tell (current session)
            write text "cd $DIR && git checkout learning1"
        end tell
    end tell
    
    -- Вкладка 2: learning2 (analysis)
    tell current window
        set t2 to (create tab with default profile)
        tell t2
            tell (current session)
                write text "cd $DIR && git checkout learning2"
            end tell
        end tell
    end tell
end tell
EOF

echo "✓ iTerm2: 2 вкладки (learning1, learning2)"

# --- VS Code: 2 окна ---
code "$DIR" --branch learning1 &
sleep 2
code "$DIR" --branch learning2 &

echo "✓ VS Code: 2 окна"
