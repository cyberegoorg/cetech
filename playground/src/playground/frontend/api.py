class FrontendApi(object):
    def __init__(self, impl):
        """
        :type impl: playground.frontend.private.app.FrontendApp
        """
        self.imp = impl

    def quit(self):
        return self.imp.quit()

    def create_window(self, name, title, parent=None, parent_area='bottom', show=True):
        return self.imp.create_window(name=name, title=title, parent=parent, parent_area=parent_area,
                                      show=show)

    def create_dock(self, name, title, window, parent=None, parent_area='bottom', show=True):
        return self.imp.create_dock(name=name, title=title, window=window, parent=parent,
                                    parent_area=parent_area,
                                    show=show)

    def create_web_content(self, name, window_name, url):
        return self.imp.create_web_content(name=name, window_name=window_name, url=url)

    def get_window(self, name):
        return self.imp.get_window(name)

    def get_dock(self, name):
        return self.imp.get_dock(name)

    def get_content(self, name):
        return self.imp.get_content(name)

    def subscribe_service(self, service_name, subscribe_clb):
        return self.imp.subscribe_service(service_name=service_name, subscribe_clb=subscribe_clb)
