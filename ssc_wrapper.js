//This is an example of a wrapper script around the windows natives core.
//this particular script defines extra helper fuctions useful for interfaceing with the game "STARBASE"

const WN = require('./build/Release/winnatives');

function keyEvent(scancode, extraFlags=0){
    return {type: FLAG.TYPE.KEYBOARD, scancode, flags: (FLAG.KEY.SCANCODE | extraFlags)}
}

const FLAG = {
    TYPE:{
        KEYBOARD:       WN.TYPE_KEYBOARD,
        MOUSE:          WN.TYPE_MOUSE
    },
    KEY:{
        EXTENDEDKEY:    WN.KFLAG_EXTENDEDKEY,
        KEYUP:          WN.KFLAG_KEYUP,
        UNICODE:        WN.KFLAG_UNICODE,
        SCANCODE:       WN.KFLAG_SCANCODE
    },
    MOUSE:{
        MOVE:           WN.MFLAG_MOVE,
        ABSOLUTE:       WN.MFLAG_ABSOLUTE,
        LEFTDOWN:       WN.MFLAG_LEFTDOWN,
        LEFTUP:         WN.MFLAG_LEFTUP,
        RIGHTDOWN:      WN.MFLAG_RIGHTDOWN,
        RIGHTUP:        WN.MFLAG_RIGHTUP,
        MIDDLEDOWN:     WN.MFLAG_MIDDLEDOWN,
        MIDDLEUP:       WN.MFLAG_MIDDLEUP
    }
}

const KEYUP = FLAG.KEY.KEYUP;
const keyevents = {
    "CTRL+V":[
        keyEvent(29),
        keyEvent(47),
        keyEvent(47, KEYUP),
        keyEvent(29, KEYUP)
    ],
    "CTRL+A":[
        keyEvent(29),
        keyEvent(30),
        keyEvent(30, KEYUP),
        keyEvent(29, KEYUP)
    ],
    "ENTER":[
        keyEvent(28),
        keyEvent(28, KEYUP)
    ],
    "DOWN+DOWN":[
        keyEvent(80, FLAG.KEY.EXTENDEDKEY),
        keyEvent(80, FLAG.KEY.EXTENDEDKEY | KEYUP),
        keyEvent(80, FLAG.KEY.EXTENDEDKEY),
        keyEvent(80, FLAG.KEY.EXTENDEDKEY | KEYUP),
    ],
    "BACKSPACE":[
        keyEvent(14),
        keyEvent(14, KEYUP)
    ]
};

const eventQueue = [];
const keyHandlers = {};

function schedule(event){
    eventQueue.push(event);
}

function schedule_key(info){
    schedule({type:"key", info});
}

function schedule_clip(info){
    schedule({type:"clip", info});
}

module.exports = {
    native:{
        setClipboard:   WN.setClipboard,
    
        sendInput:      WN.sendInput,
        getKeyState:    WN.getKeyState,
        getScanCode:    WN.getScanCode,
    
        FLAG
    },
    keyEvent, keyevents,

    schedule, schedule_key, schedule_clip,

    onShift: (key, handler) =>{
        keyHandlers[key] = handler;
    },

    handleEvent:()=>{
        let rv = 0;
        if(eventQueue.length>0){
            let event = eventQueue.shift();
            //console.log("Handling event", event);
            switch(event.type){
                case "key" : rv = WN.sendInput(event.info); break;
                case "clip": rv = WN.setClipboard(event.info); break;
            }
        } else {
            
            let keys = Object.keys(keyHandlers);
            for(let i=0; i<keys.length; i++){
                let key = keys[i];
                if(WN.getKeyState(key.charCodeAt(0))) {
                    if(WN.getKeyState(16) && WN.getKeyState(160)){
                        keyHandlers[key]();
                    }
                    break;
                }
            }
            
            rv = 1
        }
        return rv;
    },

    sendString:(string)=>{
        schedule_clip(string);
        keyevents["CTRL+V"].forEach(schedule_key);
    },

    backspace:(number=1)=>{
        for(let i=0; i<number; i++){
            keyevents["BACKSPACE"].forEach(schedule_key);
        }
    },
    
    nextLine:(ssc=false)=>{
        let eventSet = keyevents[ssc?"DOWN+DOWN":"ENTER"];
        eventSet.forEach(schedule_key);
    },

    selectLine:(ssc=true)=>{
        if(!ssc) throw "not implemented";
        keyevents["CTRL+A"].forEach(schedule_key);
    }

}
