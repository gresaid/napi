const http = require('http');
const fs = require('fs').promises;
require('path');
const querystring = require('querystring');
const {checkPrivilege} = require('./addon/build/Release/addon');

const server = http.createServer(async (req, res) => {
    const {pathname} = new URL(req.url, 'http://localhost');
    const filename = pathname === '/' ? './public/index.html' : `.${pathname}`;

    try {
        if (pathname === '/check' && req.method === 'POST') {
            const body = await parseRequestBody(req);
            const postData = querystring.parse(body);
            const username = postData.username.trim();
            const privilege = checkPrivilege();

            if (privilege === "error") {
                return handleError(res, "Ошибка в получении пользователя");
            }

            let numPrivilege = " такого пользователя не существует";

            const result = privilege.split(" ");
            for (const string of result) {
                if (string.includes(username)) {
                    const pr = string.replace(/\x00/g, '').split(":");
                    if (pr[0] === username) {
                        if (pr[1] === "0") {
                            numPrivilege = " - имеет привилегию Гостя";
                        } else if (pr[1] === "1") {
                            numPrivilege = " - имеет привилегию Пользователя";
                        } else if (pr[1] === "2") {
                            numPrivilege = " - имеет привилегию Администратора";
                        } else {
                            numPrivilege = " - не имеет привелегий";
                        }
                    }
                }
            }

            res.writeHead(200, {'Content-Type': 'text/html; charset=utf-8'});
            res.write(username + numPrivilege);
            res.write(`<br><br><button onclick="history.back()">Назад</button>`);
            return res.end();
        } else {
            const data = await fs.readFile(filename);
            res.writeHead(200, {'Content-Type': 'text/html'});
            res.write(data);
            return res.end();
        }
    } catch (error) {
        return handleError(res, error.message || "Internal Server Error");
    }
});

function parseRequestBody(req) {
    return new Promise((resolve, reject) => {
        let body = '';
        req.on('data', chunk => body += chunk.toString());
        req.on('end', () => resolve(body));
        req.on('error', reject);
    });
}

function handleError(res, message) {
    res.writeHead(500, {'Content-Type': 'text/html'});
    res.write(message);
    return res.end();
}

server.listen(3000, () => {
    console.log('Server running at http://localhost:3000/');
});
