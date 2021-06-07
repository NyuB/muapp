var https = require('https');
var http = require('http');
var fs = require('fs');

options= (msg) =>  {
    return {
        hostname: 'localhost',
        port: 5555,
        path: '/ping',
        method: 'GET',
        headers:{
            "Content-Length" : msg.length,
            "COntent-Type": "application/json"
        },
        //TLS related options
        ca: [ fs.readFileSync('ssl/srv-cert.pem') ]
    }
}

argv = process.argv.slice(2);
secure = argv[0] || "n";

if(secure === "n"){
    http.request(options("Hey"), res => {
        var data = "";
        res.on('data', d => {
            data+=d;
        });
        res.on('end', () => {
            console.log(data);
        });
        res.on('error', e => {
            console.log(e);
        })
    }).end();
}
else if(secure === "y"){
    https.request(options("Hey"), res => {
        var data = "";
        res.on('data', d => {
            data+=d;
        });
        res.on('end', () => {
            console.log(data);
        });
        res.on('error', e => {
            console.log(e);
        })
    }).end();
}

