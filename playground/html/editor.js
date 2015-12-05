var editor;

function init() {
    editor = ace.edit("editor");
    editor.setTheme("ace/theme/solarized_dark");
    editor.getSession().setMode("ace/mode/javascript");

    editor.getSession().on("change", function(e){
        editor_widget._on_change()
    });

    editor.setOptions({
        enableBasicAutocompletion: true,
        enableSnippets: true
    });

    editor.setFontSize(13)
}
