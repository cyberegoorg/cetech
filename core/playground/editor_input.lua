EditorInput = EditorInput or {}

function EditorInput:Init()
    self.delta_x = 0
    self.delta_y = 0

    self.left = false
    self.midle = false
    self.right = false

    self.keyboard = {
        up = false,
        down = false,
        left = false,
        right = false,
    }
end

function EditorInput:Move(dx, dy, left, midle, right)
    self.delta_x = dx;
    self.delta_y = dy;

    self.left = left
    self.midle = midle
    self.right = right
end

function EditorInput:ResetButtons()
    self.left = false
    self.midle = false
    self.right = false
end