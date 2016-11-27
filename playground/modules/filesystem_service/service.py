import logging
import os


class FilesystemService(object):
    def __init__(self, service_manager, logger=logging):
        self.service_manager = service_manager
        self.logger = logger.getLogger("filesystem_service")

        self.name = None
        self.project_dir = None

    @property
    def _project_service(self):
        """
        :rtype: project_service.service.ProjectService
        """
        return self.service_manager.get_service_instance("project_service")

    @property
    def _asset_service(self):
        """
        :rtype: assets.service.AssetService
        """
        return self.service_manager.get_service_instance("asset_service")

    def api_read(self, path):
        full_path = os.path.join(self._project_service.get_source_dir(), path)

        self.logger.debug("Read '%s'" % full_path)

        with open(full_path) as f:
            content = f.read()

        return content

    def api_write(self, path, content):
        full_path = os.path.join(self._project_service.get_source_dir(), path)

        self.logger.info("Write '%s'" % full_path)

        with open(full_path, 'w') as f:
            f.write(content)

        self._asset_service.api_compile_all()

    def run(self):
        pass

    def close(self):
        pass
