# KAI
Artificial Inteligence CLI powered by HuggingFace.

To do: 
- Fix memory (make memory safe).
- Load prompts and info from json config file
- Chat load and store conversations
- Other models (Summarize, translate, technical question)
- Launch with rofi
- Integrate in nvim (lua)

Estructura de la memoria: 
Lista enlazada de interacciones. Cada prompt se cargan las N últimas interacciones del chat, o un resumen
