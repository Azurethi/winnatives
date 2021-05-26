#include "defs.hh"

namespace winnatives {

    //Windows api wrappers
    napi_value method_getKeyState(napi_env env, napi_callback_info info);
    napi_value method_sendInput(napi_env env, napi_callback_info info);
    napi_value method_getScanCode(napi_env env, napi_callback_info info);
    
    napi_value method_setClipboard(napi_env env, napi_callback_info info);

    napi_value method_GetCursorPos(napi_env env, napi_callback_info info);
    napi_value method_SetCursorPos(napi_env env, napi_callback_info info);
    //--------------------
    
    napi_value init(napi_env env, napi_value exports) {

        napi_value fn;  //temp var for function/value registering

        registerFunction(env, fn, method_getKeyState,       "getKeyState"       );
        registerFunction(env, fn, method_sendInput,         "sendInput"         );
        registerFunction(env, fn, method_getScanCode,       "getScanCode"       );

        registerFunction(env, fn, method_setClipboard,      "setClipboard"      );

        registerFunction(env, fn, method_GetCursorPos,      "getCursorPos"      );
        registerFunction(env, fn, method_SetCursorPos,      "setCursorPos"      );

        //input types
        registerConstant(env, fn, INPUT_KEYBOARD,           "TYPE_KEYBOARD"     );
        registerConstant(env, fn, INPUT_MOUSE,              "TYPE_MOUSE"        );

        //keyboard flags
        registerConstant(env, fn, KEYEVENTF_EXTENDEDKEY,    "KFLAG_EXTENDEDKEY" );
        registerConstant(env, fn, KEYEVENTF_KEYUP,          "KFLAG_KEYUP"       );
        registerConstant(env, fn, KEYEVENTF_UNICODE,        "KFLAG_UNICODE"     );
        registerConstant(env, fn, KEYEVENTF_SCANCODE,       "KFLAG_SCANCODE"    );

        //mouse flags
        registerConstant(env, fn, MOUSEEVENTF_MOVE,         "MFLAG_MOVE"        );
        registerConstant(env, fn, MOUSEEVENTF_ABSOLUTE,     "MFLAG_ABSOLUTE"    );
        registerConstant(env, fn, MOUSEEVENTF_LEFTDOWN,     "MFLAG_LEFTDOWN"    );
        registerConstant(env, fn, MOUSEEVENTF_LEFTUP,       "MFLAG_LEFTUP"      );
        registerConstant(env, fn, MOUSEEVENTF_RIGHTDOWN,    "MFLAG_RIGHTDOWN"   );
        registerConstant(env, fn, MOUSEEVENTF_RIGHTUP,      "MFLAG_RIGHTUP"     );
        registerConstant(env, fn, MOUSEEVENTF_MIDDLEDOWN,   "MFLAG_MIDDLEDOWN"  );
        registerConstant(env, fn, MOUSEEVENTF_MIDDLEUP,     "MFLAG_MIDDLEUP"    );
        

        return exports;
    }

    NAPI_MODULE(NODE_GYP_MODULE_NAME, init)
    

    napi_value method_getKeyState(napi_env env, napi_callback_info info){
        
        napi_value args[1];
        size_t argc = 1;

        int32_t key;
        nc(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "method_getKeyState", "Failed to get argument");
        if(argc != 1) { napi_throw_error(env, "method_getKeyState", "Need one argument"); return nullptr;}
        nc(napi_get_value_int32(env, args[0], &key), env, "method_getKeyState", "Failed to get int32 argument");

        bool pressed = (GetAsyncKeyState(key) == -32767);

        return_int32(env, pressed, "method_getKeyState");
    }

    napi_value method_sendInput(napi_env env, napi_callback_info info){

        napi_value args[1];
        size_t argc = 1;

        nc(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "method_sendInput", "Failed to get argument");
        if(argc != 1) { napi_throw_error(env, "method_sendInput", "Need one argument"); return nullptr;}

        INPUT ip = { 0 };

        //assign properties for keyboard or mouse events
        bool hadProp;
        assignPropIfHas(env, hadProp, args[0], "type",      ip.type,        "method_sendInput");
        if(ip.type == INPUT_KEYBOARD){
            assignPropIfHas(env, hadProp, args[0], "flags",     ip.ki.dwFlags,  "method_sendInput");
            assignPropIfHas(env, hadProp, args[0], "keycode",   ip.ki.wVk,      "method_sendInput");
            assignPropIfHas(env, hadProp, args[0], "scancode",  ip.ki.wScan,    "method_sendInput");
        } else if(ip.type == INPUT_MOUSE){
            assignPropIfHas(env, hadProp, args[0], "dx",        ip.mi.dx,       "method_sendInput");
            assignPropIfHas(env, hadProp, args[0], "dy",        ip.mi.dy,       "method_sendInput");
            assignPropIfHas(env, hadProp, args[0], "flags",     ip.mi.dwFlags,  "method_sendInput");
        }
        
        UINT sent = SendInput(1, &ip, sizeof(INPUT));

        return_int32(env, sent, "method_sendInput");
    }

    napi_value method_getScanCode(napi_env env, napi_callback_info info){
        napi_value args[1];
        size_t argc = 1;

        int32_t key;
        nc(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "method_getScanCode", "Failed to get argument");
        if(argc != 1) { napi_throw_error(env, "method_getScanCode", "Need one argument"); return nullptr;}
        nc(napi_get_value_int32(env, args[0], &key), env, "method_getScanCode", "Failed to get int32 argument");

        int scancode = MapVirtualKey(LOBYTE(VkKeyScan(key)),0);

        return_int32(env, scancode, "method_getScanCode");
    }

    napi_value method_setClipboard(napi_env env, napi_callback_info info){
        napi_value args[1];
        size_t argc = 1;

        char16_t buffer[2048] = { 0 };
        size_t size = 0;

        nc(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "method_setClipboard", "Failed to get argument");
        if(argc != 1) { napi_throw_error(env, "method_setClipboard", "Need one argument"); return nullptr;}
        nc(napi_get_value_string_utf16(env, args[0], buffer, 2048, &size), env, "method_setClipboard", "Failed to get argument value");

        if(!OpenClipboard(NULL)) {
            napi_throw_error(env, "method_setClipboard", "Couldn't open clipboard"); 
            return nullptr;
        }
        
        HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (size+1) * sizeof(char16_t));
        if (hglbCopy == NULL) {
            CloseClipboard();
            napi_throw_error(env, "method_setClipboard", "Couldn't GlobalAlloc");
            return nullptr; 
        }

        char16_t* globalBuf = (char16_t*) GlobalLock(hglbCopy);
        memcpy(globalBuf, buffer, size * sizeof(char16_t));
        globalBuf[size] = 0;    //null term
        GlobalUnlock(hglbCopy);

        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, hglbCopy);
        CloseClipboard();

        return_int32(env, 1, "method_setClipboard");
    }

    napi_value method_GetCursorPos(napi_env env, napi_callback_info info){
        POINT pt = { 0 };
        BOOL success = GetCursorPos(&pt);

        napi_value returnObj;
        nc(napi_create_object(env, &returnObj), env, "method_GetCursorPos", "failed to create object");

        napi_value tmp;
        nc(napi_create_int32(env, pt.x, &tmp), env, "method_GetCursorPos", "Failed to create i32");
        nc(napi_set_named_property(env, returnObj, "x", tmp), env, "method_GetCursorPos", "failed to set obj property");
        nc(napi_create_int32(env, pt.y, &tmp), env, "method_GetCursorPos", "Failed to create i32");
        nc(napi_set_named_property(env, returnObj, "y", tmp), env, "method_GetCursorPos", "failed to set obj property");
        nc(napi_create_int32(env, success, &tmp), env, "method_GetCursorPos", "Failed to create i32");
        nc(napi_set_named_property(env, returnObj, "success", tmp), env, "method_GetCursorPos", "failed to set obj property");

        return returnObj;
    }

    napi_value method_SetCursorPos(napi_env env, napi_callback_info info){
        napi_value args[2];
        size_t argc = 2;

        int32_t xpos, ypos;
        nc(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr), env, "method_SetCursorPos", "Failed to get argument");
        if(argc != 2) { napi_throw_error(env, "method_SetCursorPos", "Need two arguments"); return nullptr;}
        nc(napi_get_value_int32(env, args[0], &xpos), env, "method_SetCursorPos", "Failed to get int32 argument");
        nc(napi_get_value_int32(env, args[1], &ypos), env, "method_SetCursorPos", "Failed to get int32 argument");

        BOOL success = SetCursorPos(xpos, ypos);

        return_int32(env, success, "method_SetCursorPos");
    }

}