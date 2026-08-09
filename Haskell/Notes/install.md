### How to install
- https://mirrors.ustc.edu.cn/help/ghcup.html, but we set the url-source at C/ghcup, not in .ghcup!
- `ghcup config set url-source https://mirrors.ustc.edu.cn/ghcup/ghcup-metadata/ghcup-latest.yaml`
- vscode set: `"haskell.manageHLS": "PATH",
    "haskell.serverExecutablePath": "C:\\ghcup\\bin\\haskell-language-server-9.14.1.exe"`
```$env:BOOTSTRAP_HASKELL_YAML = 'https://mirrors.ustc.edu.cn/ghcup/ghcup-metadata/ghcup-latest.yaml'
Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
Invoke-Command -ScriptBlock ([ScriptBlock]::Create((Invoke-WebRequest https://mirrors.ustc.edu.cn/ghcup/sh/bootstrap-haskell.ps1 -UseBasicParsing))) -ArgumentList $true```