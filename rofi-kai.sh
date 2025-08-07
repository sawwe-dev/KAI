#!/bin/bash

# Cargo las variables de entorno
source "/home/srobla/.bashrc"
source "/home/srobla/.profile"
echo $HF_API_KEY > ~/wtf.txt

# Get script directory (in case ai is local)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Ask question via Rofi
question=$(rofi -dmenu -p "Ask AI")

# Exit if cancelled
[[ -z "$question" ]] && exit 0

# Get the answer
answer="$("$SCRIPT_DIR/kai" -a "$question" 2> /home/srobla/wtf1.txt)"

# Choose how to render:
RENDER_WITH="glow"  # or: grip, marktext, typora, obsidian...

# Save to temporary Markdown file
tmpfile=$(mktemp --suffix=.md)
echo "$answer" > "$tmpfile"

# Show with markdown renderer
case "$RENDER_WITH" in
  glow)
    # Terminal markdown renderer
    echo "$answer" > /home/srobla/wtf.txt
    alacritty -e bash -c "cat $tmpfile; read"
    ;;
  grip)
    # Web server markdown viewer
    grip "$tmpfile" & xdg-open http://localhost:6419 & disown
    ;;
  marktext|typora|obsidian)
    "$RENDER_WITH" "$tmpfile" & disown
    ;;
  *)
    # Fallback: show in plain rofi
    echo "$answer" | rofi -e "$(cat)"
    ;;
esac

