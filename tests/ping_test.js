var http = require('http');
var args= process.argv.slice(2);
const PORT = args[0] || 5555;
const REMOTE = args[1] || "localhost";
options = (path) =>  {
    return {
        hostname: REMOTE,
        port: PORT,
        path: path,
        method: 'GET',
        headers:{
            "Content-Length" : 0,
            "Content-Type": "application/json"
        }
    }
}

var req = http.request(options("/ping"), res => {
    var raw = '';
    res.on('data', d => {
        raw+=d;
    });
    res.on('end', () => {
        var pong = JSON.parse(raw);
        if(pong.pong !== 42){
            process.exit(-1);
        }
        else {
            process.exit(0);
        }
    });
    res.on('error', _ => {
        process.exit(-1);
    });
});
req.write("");
req.end();