import functools
import logging
import os

from watchdog.events import FileSystemEventHandler
from watchdog.observers import Observer


class FSEventHandler(FileSystemEventHandler):
    def __init__(self, publish, project_service):
        self.publish = publish
        self.project_service = project_service

    def on_moved(self, event):
        super(FSEventHandler, self).on_moved(event)
        what = 'directory' if event.is_directory else 'file'

    def on_created(self, event):
        super(FSEventHandler, self).on_created(event)
        what = 'directory' if event.is_directory else 'file'

    def on_deleted(self, event):
        super(FSEventHandler, self).on_deleted(event)
        what = 'directory' if event.is_directory else 'file'

    def on_modified(self, event):
        super(FSEventHandler, self).on_modified(event)
        msg_type = 'dir_modified' if event.is_directory else 'file_modified'

        project_dir = self.project_service.api_get_project_dir()
        path = event.src_path.replace(project_dir, "")[1:]

        self.publish(dict(msg_type=msg_type, path=path))


class FilesystemService(object):
    SERVICE_NAME = "filesystem_service"

    def __init__(self, service_manager, logger=logging):
        self.service_manager = service_manager
        self.logger = logger.getLogger(self.SERVICE_NAME)

        self.name = None
        self.project_dir = None

        self.publish = functools.partial(self.service_manager.publish, self.SERVICE_NAME, "event")

        self.event_handler = FSEventHandler(self.publish, self._project_service)
        self.service_manager.subscribe_service("project_service", self._subscribe_project_service)

    def _subscribe_project_service(self, msg):
        self.logger.debug(msg)

        if msg['msg_type'] == 'project_opened':
            self.observer = Observer()
            self.observer.schedule(self.event_handler, self._project_service.project_dir, recursive=True)
            self.observer.schedule(self.event_handler, self._project_service.core_dir, recursive=True)
            self.observer.start()

        elif msg['msg_type'] == 'project_closed':
            self.observer.stop()
            self.observer.join()

    def api_read(self, path):
        """
        :type path: str
        """

        path = path[:-1] if path.endswith("/") else path

        full_path = os.path.join(self._project_service.api_get_project_dir(), path)

        self.logger.debug("Read '%s'" % full_path)

        with open(full_path) as f:
            content = f.read()

        return content

    def api_write(self, path, content):
        """
        :type path: str
        :type content: str
        """

        path = path[:-1] if path.endswith("/") else path

        full_path = os.path.join(self._project_service.api_get_project_dir(), path)

        self.logger.info("Write '%s'" % full_path)

        with open(full_path, 'w') as f:
            f.write(content)

    def run(self):
        pass

    def close(self):
        self.observer.stop()
        self.observer.join()

    @property
    def _project_service(self):
        """
        :rtype: core.project_service.service.ProjectService
        """
        return self.service_manager.get_service_instance("project_service")

    @property
    def _engine_service(self):
        """
        :rtype: engine_service.service.EngineService
        """
        return self.service_manager.get_service_instance("engine_service")

    @property
    def _asset_service(self):
        """
        :rtype: assets.service.AssetService
        """
        return self.service_manager.get_service_instance("asset_service")
