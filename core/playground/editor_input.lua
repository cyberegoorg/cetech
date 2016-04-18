EditorInput = EditorInput or {}

function EditorInput:Init()
    self.mouse = {
        dx = 0,
        dy = 0,

        left = false,
        right = false,
        midle = false
    }

    self.keyboard = {
        up = false,
        down = false,
        left = false,
        right = false,
    }
end

function EditorInput:Move(dx, dy, left, midle, right)
    self.mouse.dx = dx;
    self.mouse.dy = dy;

    self.mouse.left = left
    self.mouse.midle = midle
    self.mouse.right = right
end

function EditorInput:ResetButtons()
    self.mouse.left = false
    self.mouse.midle = false
    self.mouse.right = false
end