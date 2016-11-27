///<reference path="ace/ace.d.ts"/>
import {PlaygroundRPC} from "./lib/playground/playground_rpc";


export class ScriptEditor {
    editor: AceAjax.Editor;

    rpc: PlaygroundRPC;
    path: string;

    SUFIX_2_MODE = {
        'lua': 'ace/mode/lua',
        'sc': 'ace/mode/c_cpp',
        'sh': 'ace/mode/c_cpp',
        '': 'ace/mode/yaml'
    };

    constructor() {
        this.path = window.location.search.substr(1);

        this.rpc = new PlaygroundRPC();

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
                this.save()
            },
            bindKey: {win: "ctrl-s"}
        });

        this.connect_to_cetech();

    }

    save() {
        this.rpc.callService("filesystem_service", "write", {
            path: this.path,
            content: this.editor.getValue()
        }, (msg: any) => {

        });
    }

    private connect_to_cetech() {
        this.rpc.close();

        this.rpc.connect("ws://localhost:8888", () => {
            this.rpc.callService("filesystem_service", "read", {
                path: this.path,
            }, (msg: any) => {
                this.editor.setValue("");
                this.editor.setValue(msg.response, -1);
                const type = this.path.split(".").pop();

                if (this.SUFIX_2_MODE.hasOwnProperty(type)) {
                    this.editor.getSession().setMode(this.SUFIX_2_MODE[type]);
                } else {
                    this.editor.getSession().setMode(this.SUFIX_2_MODE['']);
                }

            });
        });
    };
}
