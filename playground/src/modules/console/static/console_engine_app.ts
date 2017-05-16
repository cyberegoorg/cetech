///<reference path="../../lib/static/ace/ace.d.ts"/>
import {EngineRPC, EngineSubscriber} from "../../lib/static/playground/engine_rpc";

declare var msgpack: any;

export class ConsoleEngineApp {
    editor: AceAjax.Editor;
    editor2: AceAjax.Editor;

    rpc: EngineRPC;
    sub: EngineSubscriber;

    level_to_class = {
        info: "info",
        warning: "warning",
        error: "danger",
        debug: "success"
    };

    level_to_icon = {
        info: "glyphicon-info-sign",
        warning: "glyphicon-warning-sign",
        error: "glyphicon-remove-sign",
        debug: "glyphicon-ok-sign"
    };

    constructor() {
        this.rpc = new EngineRPC();
        this.sub = new EngineSubscriber();

        this.sub.subscribe((msg): void => {

            const log_msg = msg;

            const table = <HTMLTableElement>document.getElementById("log_table");
            const row = table.insertRow(1);


            row.className += this.level_to_class[log_msg['level']];

            const level = row.insertCell(0);
            const where = row.insertCell(1);
            const worker_id = row.insertCell(2);
            const message = row.insertCell(3);

            level.innerHTML = "<span aria-hidden=\"true\" class=\"glyphicon  " + this.level_to_icon[log_msg['level']] + "\"></span>";
            where.innerHTML = log_msg['where'];
            worker_id.innerHTML = log_msg['worker_id'];
            message.innerHTML = log_msg['msg'];
        });

        this.editor = ace.edit("editor");
        this.editor.$blockScrolling = Infinity;
        this.editor.setTheme("ace/theme/solarized_dark");
        this.editor.getSession().setMode("ace/mode/lua");

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
        this.editor2.$blockScrolling = Infinity;
        this.editor2.setTheme("ace/theme/solarized_dark");
        this.editor2.getSession().setMode("ace/mode/json");

        this.editor2.setFontSize("15px");


        let btn = <HTMLButtonElement>document.getElementById("btn_connect");
        btn.onclick = () => {
            this.connect_to_cetech()
        };

        btn = <HTMLButtonElement>document.getElementById("btn_send");
        btn.onclick = () => {
            this.send_cmd()
        };
    }

    send_cmd() {
        const script = this.editor.getValue();

        this.rpc.call("lua_system.execute",{
            script: script
        }, (msg: any) => {
            this.editor2.setValue(JSON.stringify(msg, null, 2));
        });
    }

    connect_to_cetech() {
        this.rpc.close();
        this.sub.close();

        const cetech_url = <HTMLInputElement>document.getElementById("cetech_url");

        this.rpc.connect(cetech_url.value);
        this.sub.connect("ws://localhost:4445")
    };
}
