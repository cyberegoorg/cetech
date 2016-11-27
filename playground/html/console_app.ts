///<reference path="ace/ace.d.ts"/>

export var msgpack: any;


export class ConsoleApp {
    editor: AceAjax.Editor;
    editor2: AceAjax.Editor;

    rpc_ws: WebSocket;
    log_ws: WebSocket;

    constructor() {
        this.editor = ace.edit("editor");
        this.editor.$blockScrolling = Infinity;
        this.editor.setTheme("ace/theme/solarized_dark");
        this.editor.getSession().setMode("ace/mode/lua");


        // editor.getSession().on("change", function(e){
        // });

        this.editor.setOptions({
            enableBasicAutocompletion: true,
            enableSnippets: true,
        });
        this.editor.setFontSize("15px");

        this.editor.commands.addCommand({
            name: "send",
            exec: () => {
                this.send_cmd()
            },
            bindKey: {win: "alt-enter"}
        });

        this.editor2 = ace.edit("editor2");
        this.editor.$blockScrolling = Infinity;
        this.editor2.setTheme("ace/theme/solarized_dark");
        this.editor2.getSession().setMode("ace/mode/json");

        this.editor2.setFontSize("15px");

        var btn = <HTMLButtonElement>document.getElementById("btn_connect");
        btn.onclick = () => {
            this.connect_to_cetech()
        };

        var btn = <HTMLButtonElement>document.getElementById("btn_send");
        btn.onclick = () => {
            this.send_cmd()
        };

    }

    send_cmd() {
        const script = this.editor.getValue();

        const msg = msgpack.encode({
            name: "lua_system.execute",
            args: {
                script: script
            }
        });

        const header = new Uint8Array([255, 255, 255, 255]);
        const tmp = new Uint8Array(header.byteLength + msg.length);

        tmp.set(header, 0);
        tmp.set(msg, header.byteLength);

        this.rpc_ws.send(tmp.buffer);
    }

    parse_data(events: any) {
        this.editor2.setValue(JSON.stringify(events, null, 2));
    }

    connect_to_cetech() {
        if (this.rpc_ws != null) {
            this.rpc_ws.close();
        }

        if (this.log_ws != null) {
            this.log_ws.close();
        }

        const cetech_url = <HTMLInputElement>document.getElementById("cetech_url");

        this.rpc_ws = new WebSocket(cetech_url.value, "rep.sp.nanomsg.org");

        this.rpc_ws.binaryType = "arraybuffer";
        this.rpc_ws.onopen = function () {
            console.log("opend");
        };

        this.rpc_ws.onclose = function () {
            console.log("closed");
        };

        this.rpc_ws.onmessage = (evt): void => {
            const data = new Uint8Array(evt.data);

            const msg_data = data.subarray(4);

            const events = msgpack.decode(msg_data);
            this.parse_data(events);
        };

        this.log_ws = new WebSocket("ws://localhost:4445", "pub.sp.nanomsg.org");

        this.log_ws.binaryType = "arraybuffer";
        this.log_ws.onopen = function () {
            console.log("opend");
        };

        this.log_ws.onclose = function () {
            console.log("closed");
        };

        this.log_ws.onmessage = function (evt) {
            const msg_data = new Uint8Array(evt.data);//.subarray(4);

            const events = msgpack.decode(msg_data);

            const table = <HTMLTableElement>document.getElementById("log_table");
            const row = table.insertRow(1);

            const level_to_class =
                {
                    info: "info",
                    warning: "warning",
                    error: "danger",
                    debug: "success"
                };

            const level_to_icon =
                {
                    info: "glyphicon-info-sign",
                    warning: "glyphicon-warning-sign",
                    error: "glyphicon-remove-sign",
                    debug: "glyphicon-ok-sign"
                };

            row.className += level_to_class[events['level']];

            const level = row.insertCell(0);
            const where = row.insertCell(1);
            const worker_id = row.insertCell(2);
            const message = row.insertCell(3);

            level.innerHTML = "<span aria-hidden=\"true\" class=\"glyphicon  " + level_to_icon[events['level']] + "\"></span>";
            where.innerHTML = events['where'];
            worker_id.innerHTML = events['worker_id'];
            message.innerHTML = events['msg'];

            //console.log(events)
        };
    };
}