///<reference path="../../lib/static/ace/ace.d.ts"/>
import {PlaygroundRPC, PlaygroundSubscriber} from "../../lib/static/playground/playground_rpc";


export class ScriptEditor {
    editor: AceAjax.Editor;

    rpc: PlaygroundRPC;
    sub: PlaygroundSubscriber;
    path: string;
    saving_in_progres: boolean;

    SUFIX_2_MODE = {
        'lua': 'ace/mode/lua',
        'sc': 'ace/mode/c_cpp',
        'sh': 'ace/mode/c_cpp',
        '': 'ace/mode/yaml'
    };

    constructor() {
        this.path = window.location.search.substr(1);

        this.rpc = new PlaygroundRPC();
        this.sub = new PlaygroundSubscriber();

        this.saving_in_progres = false;

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

        this.sub.subcribeService("filesystem_service", (msg): void => {
            if (msg.msg_type == 'event') {
                msg = msg.msg;

                if (msg.msg_type == 'file_modified') {
                    if (msg.path == this.path) {
                        if (this.saving_in_progres) {
                            this.saving_in_progres = false;
                        } else {
                            this.open(this.path);
                        }
                    }
                }
            }
        })
    }

    save() {
        this.saving_in_progres = true;
        this.rpc.callService("filesystem_service", "write", {
            path: this.path,
            content: this.editor.getValue()
        }, (msg: any) => {
        });
    }

    open(path: string) {
        this.rpc.callService("filesystem_service", "read", {
            path: this.path,
        }, (msg: any) => {
            const curent_pos = this.editor.getCursorPosition();

            this.editor.setValue(msg.response, -1);
            const type = path.split(".").pop();

            if (this.SUFIX_2_MODE.hasOwnProperty(type)) {
                this.editor.getSession().setMode(this.SUFIX_2_MODE[type]);
            } else {
                this.editor.getSession().setMode(this.SUFIX_2_MODE['']);
            }

            this.editor.moveCursorToPosition(curent_pos);
        });
    }

    private connect_to_cetech() {
        this.rpc.close();
        this.sub.close();

        this.rpc.connect("ws://localhost:8888", () => {
            this.open(this.path);
        });

        this.sub.connect("ws://localhost:8889")
    };
}
