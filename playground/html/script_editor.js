System.register(["./lib/playground/playground_rpc"], function(exports_1, context_1) {
    "use strict";
    var __moduleName = context_1 && context_1.id;
    var playground_rpc_1;
    var ScriptEditor;
    return {
        setters:[
            function (playground_rpc_1_1) {
                playground_rpc_1 = playground_rpc_1_1;
            }],
        execute: function() {
            ScriptEditor = (function () {
                function ScriptEditor() {
                    var _this = this;
                    this.SUFIX_2_MODE = {
                        'lua': 'ace/mode/lua',
                        'sc': 'ace/mode/c_cpp',
                        'sh': 'ace/mode/c_cpp',
                        '': 'ace/mode/yaml'
                    };
                    this.path = window.location.search.substr(1);
                    this.rpc = new playground_rpc_1.PlaygroundRPC();
                    this.editor = ace.edit("editor");
                    this.editor.$blockScrolling = Infinity;
                    this.editor.setTheme("ace/theme/solarized_dark");
                    this.editor.getSession().setMode("ace/mode/lua");
                    this.editor.setOptions({
                        enableBasicAutocompletion: true,
                        enableSnippets: true
                    });
                    this.editor.setFontSize("15px");
                    this.editor.commands.addCommand({
                        name: "send",
                        exec: function () {
                            _this.save();
                        },
                        bindKey: { win: "ctrl-s" }
                    });
                    this.connect_to_cetech();
                }
                ScriptEditor.prototype.save = function () {
                    this.rpc.callService("filesystem_service", "write", {
                        path: this.path,
                        content: this.editor.getValue()
                    }, function (msg) {
                    });
                };
                ScriptEditor.prototype.connect_to_cetech = function () {
                    var _this = this;
                    this.rpc.close();
                    this.rpc.connect("ws://localhost:8888", function () {
                        _this.rpc.callService("filesystem_service", "read", {
                            path: _this.path
                        }, function (msg) {
                            _this.editor.setValue("");
                            _this.editor.setValue(msg.response, -1);
                            var type = _this.path.split(".").pop();
                            if (_this.SUFIX_2_MODE.hasOwnProperty(type)) {
                                _this.editor.getSession().setMode(_this.SUFIX_2_MODE[type]);
                            }
                            else {
                                _this.editor.getSession().setMode(_this.SUFIX_2_MODE['']);
                            }
                        });
                    });
                };
                ;
                return ScriptEditor;
            }());
            exports_1("ScriptEditor", ScriptEditor);
        }
    }
});
//# sourceMappingURL=script_editor.js.map