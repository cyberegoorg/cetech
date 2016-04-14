EditorMouse = EditorMouse or {}

function EditorMouse:Init()
    self.last_x = 0
    self.last_y = 0

    self.delta_x = 0
    self.delta_y = 0

    self.left = false
    self.midle = false
    self.right = false
end

function EditorMouse:Move(x, y, left, midle, right)
    self.delta_x = self.last_x - x;
    self.delta_y = self.last_y - y;

    self.last_x = x;
    self.last_y = y;

    self.left = left
    self.midle = midle
    self.right = right
end

function EditorMouse:ResetButtons()
    self.left = false
    self.midle = false
    self.right = false
end