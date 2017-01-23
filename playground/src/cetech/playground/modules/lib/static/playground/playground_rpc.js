System.register([], function(exports_1, context_1) {
    "use strict";
    var __moduleName = context_1 && context_1.id;
    var PlaygroundRPC, PlaygroundSubscriber;
    return {
        setters:[],
        execute: function() {
            PlaygroundRPC = (function () {
                function PlaygroundRPC() {
                    this.connected = false;
                    this.response_callbacks = {};
                }
                PlaygroundRPC.prototype.connect = function (url, onopen) {
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
                PlaygroundRPC.prototype.close = function () {
                    if (this.rpc_ws) {
                        this.rpc_ws.close();
                    }
                };
                PlaygroundRPC.prototype.callService = function (service_name, fce_name, args, on_ok) {
                    var request = {
                        service: service_name,
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
                PlaygroundRPC.prototype._parseMessage = function (msg) {
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
                return PlaygroundRPC;
            }());
            exports_1("PlaygroundRPC", PlaygroundRPC);
            PlaygroundSubscriber = (function () {
                function PlaygroundSubscriber() {
                    this.connected = false;
                    this.responce_callbacks = {};
                }
                PlaygroundSubscriber.prototype.connect = function (url) {
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
                        var msg_data = new Uint8Array(msg.data);
                        var unpack_msg = msgpack.decode(msg_data);
                        if (_this.responce_callbacks.hasOwnProperty(unpack_msg.service)) {
                            _this.responce_callbacks[unpack_msg.service](unpack_msg);
                        }
                    };
                };
                PlaygroundSubscriber.prototype.close = function () {
                    if (this.sub_ws) {
                        this.sub_ws.close();
                    }
                };
                PlaygroundSubscriber.prototype.subcribeService = function (service_name, on_msg) {
                    this.responce_callbacks[service_name] = on_msg;
                };
                return PlaygroundSubscriber;
            }());
            exports_1("PlaygroundSubscriber", PlaygroundSubscriber);
        }
    }
});
//# sourceMappingURL=playground_rpc.js.map