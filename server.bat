@echo off
cd addon
call node-gyp configure build
cd ..
call node index.js
timeout 2 >nul
start http://localhost:3000