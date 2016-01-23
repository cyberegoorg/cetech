import json
import time

from PyQt5.QtCore import QUrl, pyqtSlot, QDir
from PyQt5.QtWebKit import QWebSettings
from PyQt5.QtWidgets import QFrame

from playground.ui.profilerwidget import Ui_ProfilerWidget


class ProfilerWidget(QFrame, Ui_ProfilerWidget):
    def __init__(self, api):
        super(ProfilerWidget, self).__init__()
        self.setupUi(self)

        self.api = api
        #self.api.register_handler('debug_event', self.debug_event)

        page = self.profile_webview.page()
        page.settings().setAttribute(QWebSettings.DeveloperExtrasEnabled, True)

        self.profile_webview.page().mainFrame().javaScriptWindowObjectCleared.connect(
            self.populateJavaScriptWindowObject)
        self.profile_webview.page().mainFrame().setUrl(QUrl("file://%s/html/profiler.html" % QDir.currentPath()))

        self.data = {}

        self.las_frame_id = 0

        self.frame_data = {}
        self.profile_data = {}

    def populateJavaScriptWindowObject(self):
        self.profile_webview.page().mainFrame().addToJavaScriptWindowObject('RecordEventWidget', self)

    @pyqtSlot(int, result=str)
    def get_data(self, frame_id):
        if frame_id not in self.profile_data:
            return "[]"

        datas = []
        counter = 0
        for frame_id, data in self.profile_data.items():
            if frame_id < 2:
                continue

            if counter > 100:
                break

            datas.extend(data)
            counter += 1

        #pprint(self.profile_data[frame_id])
        #print(datas)

        data = json.dumps(datas)
        return data

    def debug_event(self, events, **kwargs):
        fid = 0

        for event in events:
            etype = event['etype']

            if etype == 'EVENT_RECORD_FLOAT':
                name = event['name']
                value = event['value']

                if name not in self.data:
                    self.data[name] = []

                self.data[name].append({'time': int(time.time()), 'value': float(value)})

            elif etype == 'EVENT_BEGIN_FRAME':
                t = event['time'], event['time_ns']
                frame_id = event['frame_id']

                if frame_id not in self.frame_data:
                    self.frame_data[frame_id] = {}
                    self.profile_data[frame_id] = []

            elif etype == 'EVENT_END_FRAME':
                t = event['time'], event['time_ns']
                frame_id = event['frame_id']

                #            print("     end frame", kwargs)

                # print("\n")
                # pprint(self.profile_data)
                # print("\n")

                self.frame_data[frame_id]["end"] = t

                self.las_frame_id = self.las_frame_id if self.las_frame_id > frame_id else frame_id

            elif etype == 'EVENT_SCOPE':
                name = event['name']
                worker_id = event['worker_id']
                t_s = event['start'], event['start_ns']
                t_e = event['end'], event['end_ns']
                frame_id = event['frame_id']
                depth = event['depth']

                fid = frame_id

                if frame_id not in self.profile_data:
                    self.profile_data[frame_id] = []

                def diff_time(start, end):
                    temp = [0, 0]
                    if (end[1] - start[1]) < 0:
                        temp[0] = end[0] - start[0] - 1
                        temp[1] = 1000000000 + end[1] - start[1]
                    else:
                        temp[0] = end[0] - start[0]
                        temp[1] = end[1] - start[1]

                    return temp

                delta_ms = diff_time(t_s, t_e)[1] / 1000000
                label = "{0}: %fms, depth: %d".format(name, delta_ms, depth)

                self.profile_data[frame_id].append({
                    "content": label,
                    "title": label,
                    "start": (t_s[0] * 1000) + (t_s[1] / 1000000),
                    "end": (t_e[0] * 1000) + (t_e[1] / 1000000),
                    "group": worker_id,
                    "depth": depth,
                })
