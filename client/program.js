const SerialPort = require('serialport');
const fs = require("fs");
const readlineSync = require('readline-sync');
const { error } = require('console');

const input = fs.readdirSync('./input');
let inputFile = "./input/"+input[0];
console.log(process.argv, 'howdy',process.argv.includes('-i'));
if (process.argv.includes('-i')){
    const index = process.argv.indexOf('-i');
    inputFile = process.argv[index+1];
} else{
    if (input.length>1){
        console.log("which  firmware would you like to flash?");
        for(i=0;i<input.length;i++){
            console.log((i+1)+":"+input[i]);
        }
        inputFile = readlineSync.question(":")-1;
        if(inputFile>input.length || inputFile<0){
            throw new Error("not in option range");
        }
        inputFile = "./input/"+input[inputFile]
    } else if(input.length == 0){
        console.log("\x1b[31mNo input file(s) detected!\nShuting down\n\x1b[0m");
        return;
    }
}

console.log("\x1b[90mfile selected: "+inputFile+"\x1b[0m")
let portPath = "COM3"
let port;
if (process.argv.includes('-setCom')){
    const index = process.argv.indexOf('-setCom');
    portPath = process.argv[index+1];
}
while(true){
    try{
        port = new SerialPort.SerialPort({
            baudRate: 115200,
            path: portPath,
            autoOpen: false
        }, false);
        break;
    } catch(error){
        console.log("Couldnt find pico at COM port: " + portPath+ "\n\x1b[0m");
        portPath = readlineSync.question("\x1b[31mWhat port is the pico connected to?\n")
    }
}


console.log("\x1b[90mUsing COM Port: "+portPath+"\x1b[0m");
    

const waitForData = () => {
    return new Promise((resolve, reject) => {
        const handler = data => {
            clearTimeout(timeout);
            resolve(data.toString("utf-8"));
        };

        let timeout = setTimeout(() => {
            console.log("\x1b[31m\nPico did not repsond!\nMake sure you are using the right COM port\nCOM used: "+portPath+" use'node program.js -setCom $COMPORT' \n\x1b[0m",);
            port.removeListener("data", handler);
            reject("Response timeout!");
        }, 5000);

        port.once("data", handler);
        
    }); 
};
let data;
try{
    data = fs.readFileSync(inputFile, { encoding: "hex" }).match(/.{1,2}/g).map(h => "0x" + h);
} catch(err){
    console.log("\x1b[31m\nfile: '"+inputFile+"' dosnt exist!\nplease run without -i or chose a vaild file!\n\x1b[0m")
    throw err;
}

port.open((async (err) => {
    if(err) throw err;

    console.log("\x1b[32mSerial connection port open!\x1b[0m");

    port.write("?");
    if ((await waitForData()) !== "READY") {
        console.log(`Programmer did not reply with ready signal!\n
            Make sure you are using the right COM port\n
            COM used:`+portPath+" use 'node program.js -asignCom' or 'node program.js -setCom $COMPORT' ");
    }

    console.log("Sending over program bytes in hex..");

    for (const c of data) {
        port.write(c + " ");
        if ((await waitForData().catch(() => null)) !== c) {
            console.log("Programmer did not readback byte correctly!");
            process.exit();
        }
    }

    console.log("\x1b[32mAll program bytes sent over!\n\x1b[0m");

    port.on("data", d => {
        if (d.toString("hex") === "0d0a") return;
        if (d.toString("utf-8").startsWith("FINISH")) {
            console.log("\x1b[32m\nProgrammer has notified me that flashing is done - goodbye!\n\x1b[0m");
            process.exit();
        }

        console.log("Message from programmer:", d.toString("utf-8"));
    });

    // Send finish byte.
    port.write(new Uint8Array([13]));
}));

