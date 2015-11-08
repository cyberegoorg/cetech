var editor

function init() {
    editor = ace.edit("editor");
    editor.setTheme("ace/theme/monokai");
    editor.getSession().setMode("ace/mode/javascript");

    editor.setOptions({
        enableBasicAutocompletion: true,
         enableSnippets: true
    })

    editor.setFontSize(13)
}

