from mapobj import Map
from network import Network
from graphics import Window
from screens import MapScreen


class Game:
    def __init__(self):
        self.network = Network()
        self.map = Map(self)
        self.window = Window(self)
        self.main_loop_running = False

    def run(self):
        if self.network.check_file_data():
            MapScreen(self.window, self.map).open_()
        self.main_loop_running = True
        while self.main_loop_running:
            self.window.iteration()
            self.network.iteration()

    def stop(self):
        self.main_loop_running = False
