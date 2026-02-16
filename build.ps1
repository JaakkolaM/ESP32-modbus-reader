$env:IDF_PATH = "C:\Users\jaakk\esp\v5.1.6\esp-idf"
& "C:\Users\jaakk\esp\v5.1.6\esp-idf\export.ps1"
idf.py fullclean
idf.py set-target esp32c3
idf.py build
Read-Host -Prompt "Press Enter to continue"
