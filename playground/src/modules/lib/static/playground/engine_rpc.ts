declare var msgpack: any;

export class EngineRPC {
    private rpc_ws: WebSocket;
    private connected: boolean = false;
    private response_callbacks = {};

    constructor() {
    }

    connect(url: string, onopen = null) {
        this.rpc_ws = new WebSocket(url, "rep.sp.nanomsg.org");
        this.rpc_ws.binaryType = "arraybuffer";
        this.rpc_ws.onopen = () => {
            this.connected = true;

            if (onopen) {
                onopen()
            }

        };

        this.rpc_ws.onclose = () => {
            this.connected = false;
        };

        this.rpc_ws.onmessage = (msg): void => {
            const data = new Uint8Array(msg.data);
            const msg_data = data.subarray(4);
            const unpack_msg = msgpack.decode(msg_data);

            this._parseMessage(unpack_msg);
        };
    }

    close() {
        if (this.rpc_ws) {
            this.rpc_ws.close();
            this.rpc_ws = null;
        }
    }

    call(fce_name: string, args: any, on_ok: (msg: any) => void) {
        const request = {
            name: fce_name,
            args: args,
            id: Math.random()
        };

        this.response_callbacks[request.id] = {
            on_ok: on_ok
        };

        const msg = msgpack.encode(request);

        const header = new Uint8Array([255, 255, 255, 255]);
        let tmp = new Uint8Array(header.byteLength + msg.length);

        tmp.set(header, 0);
        tmp.set(msg, header.byteLength);

        this.rpc_ws.send(tmp.buffer);
    }

    private _parseMessage(msg: any) {
        console.log(msg);
        if (!msg.hasOwnProperty("response") ||
            ((msg.response != null) && msg.response.hasOwnProperty("error"))) {
            console.error("RPC response error: " + msg);
        } else {
            this.response_callbacks[msg.id].on_ok(msg);
        }

        delete this.response_callbacks[msg.id];
    }
}

export class EngineSubscriber {
    private sub_ws: WebSocket;
    private connected: boolean = false;
    private on_log = null;

    constructor() {
    }

    connect(url: string) {
        this.sub_ws = new WebSocket(url, "pub.sp.nanomsg.org");
        this.sub_ws.binaryType = "arraybuffer";
        this.sub_ws.onopen = () => {
            this.connected = true;
        };

        this.sub_ws.onclose = () => {
            this.connected = false;
        };

        this.sub_ws.onmessage = (msg): void => {
            if (this.on_log == null) {
                return;
            }

            const msg_data = new Uint8Array(msg.data);
            const unpack_msg = msgpack.decode(msg_data);

            this.on_log(unpack_msg);
        };
    }

    close() {
        if (this.sub_ws) {
            this.sub_ws.close()
        }
    }

    subscribe(on_log: (msg: any) => void) {
        this.on_log = on_log
    }
}
