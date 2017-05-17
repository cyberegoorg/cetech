"use strict";
exports.__esModule = true;
var EngineRPC = (function () {
    function EngineRPC() {
        this.connected = false;
        this.response_callbacks = {};
    }
    EngineRPC.prototype.connect = function (url, onopen) {
        var _this = this;
        if (onopen === void 0) { onopen = null; }
        this.rpc_ws = new WebSocket(url, "rep.sp.nanomsg.org");
        this.rpc_ws.binaryType = "arraybuffer";
        this.rpc_ws.onopen = function () {
            _this.connected = true;
            if (onopen) {
                onopen();
            }
        };
        this.rpc_ws.onclose = function () {
            _this.connected = false;
        };
        this.rpc_ws.onmessage = function (msg) {
            var data = new Uint8Array(msg.data);
            var msg_data = data.subarray(4);
            var unpack_msg = msgpack.decode(msg_data);
            _this._parseMessage(unpack_msg);
        };
    };
    EngineRPC.prototype.close = function () {
        if (this.rpc_ws) {
            this.rpc_ws.close();
            this.rpc_ws = null;
        }
    };
    EngineRPC.prototype.call = function (fce_name, args, on_ok) {
        var request = {
            name: fce_name,
            args: args,
            id: Math.random()
        };
        this.response_callbacks[request.id] = {
            on_ok: on_ok
        };
        var msg = msgpack.encode(request);
        var header = new Uint8Array([255, 255, 255, 255]);
        var tmp = new Uint8Array(header.byteLength + msg.length);
        tmp.set(header, 0);
        tmp.set(msg, header.byteLength);
        this.rpc_ws.send(tmp.buffer);
    };
    EngineRPC.prototype._parseMessage = function (msg) {
        console.log(msg);
        if (!msg.hasOwnProperty("response") ||
            ((msg.response != null) && msg.response.hasOwnProperty("error"))) {
            console.error("RPC response error: " + msg);
        }
        else {
            this.response_callbacks[msg.id].on_ok(msg);
        }
        delete this.response_callbacks[msg.id];
    };
    return EngineRPC;
}());
exports.EngineRPC = EngineRPC;
var EngineSubscriber = (function () {
    function EngineSubscriber() {
        this.connected = false;
        this.on_log = null;
    }
    EngineSubscriber.prototype.connect = function (url) {
        var _this = this;
        this.sub_ws = new WebSocket(url, "pub.sp.nanomsg.org");
        this.sub_ws.binaryType = "arraybuffer";
        this.sub_ws.onopen = function () {
            _this.connected = true;
        };
        this.sub_ws.onclose = function () {
            _this.connected = false;
        };
        this.sub_ws.onmessage = function (msg) {
            if (_this.on_log == null) {
                return;
            }
            var msg_data = new Uint8Array(msg.data);
            var unpack_msg = msgpack.decode(msg_data);
            _this.on_log(unpack_msg);
        };
    };
    EngineSubscriber.prototype.close = function () {
        if (this.sub_ws) {
            this.sub_ws.close();
        }
    };
    EngineSubscriber.prototype.subscribe = function (on_log) {
        this.on_log = on_log;
    };
    return EngineSubscriber;
}());
exports.EngineSubscriber = EngineSubscriber;
//# sourceMappingURL=engine_rpc.js.map