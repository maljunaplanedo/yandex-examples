import random
from network import INetworkEventSubscriber
import copy
from abc import abstractmethod
import screens


# Composite pattern
class IMapObjInfoConvertible:
    @abstractmethod
    def get_info(self):
        pass

    @abstractmethod
    def reset_from_info(self, info):
        pass


class Player(IMapObjInfoConvertible):
    def __init__(self, map_):
        self.gold = 500
        self.map = map_

    def get_info(self):
        info = {'gold': self.gold}
        return info

    def reset_from_info(self, info):
        self.gold = info['gold']


class Fortress(IMapObjInfoConvertible):
    SHOP_SIZE = 6
    FORTRESS_PROFIT = 100

    def __init__(self, game, x, y):
        self.game = game
        self.guest = None
        self.garrison = None
        self.shop = []
        self.master = None
        self.x = x
        self.y = y
        self.generate_shop()

    def get_info(self):
        master = self.game.map.get_player_code(self.master)

        info = {'guest': self.game.map.get_index_of_squad(self.guest),
                'garrison': self.game.map.get_index_of_squad(self.garrison),
                'shop': [],
                'master': master,
                'x': self.x,
                'y': self.y}

        for i in self.shop:
            info['shop'].append([i[0].get_info(), i[1]])

        return info

    def reset_from_info(self, info):
        self.guest = self.game.map.get_squad_by_index(info['guest'])
        self.garrison = self.game.map.get_squad_by_index(info['garrison'])
        self.master = self.game.map.get_player_by_code(info['master'])
        self.shop = []

        for i in info['shop']:
            self.shop.append((Soldier(), i[1]))
            self.shop[-1][0].reset_from_info(i[0])

    def open_fortress_menu(self):
        screens.FortressScreen(self.game.window, self).open_()

    def change_master(self, master):
        self.master = master

    def generate_shop(self):
        for i in range(self.SHOP_SIZE):
            soldier = Soldier(None,
                              random.randint(Soldier.MIN_ARMOR, Soldier.MAX_ARMOR),
                              random.randint(Soldier.MIN_ATTACK, Soldier.MAX_ATTACK)
                              )
            self.shop.append((soldier, soldier.cost()))

    def recruit_soldier(self, index):
        soldier_cost = self.shop[index]
        if soldier_cost[1] > self.guest.player.gold:
            return False
        self.guest.player.gold -= soldier_cost[1]
        self.guest.add_soldier(copy.copy(soldier_cost[0]))
        return True

    def move_soldier(self, index, to_guest):
        from_, to_ = (self.garrison, self.guest) if to_guest else (self.guest, self.garrison)
        to_.add_soldier(from_.soldiers[index])
        from_.soldiers.pop(index)

    def close_fortress_menu(self):
        screens.MapScreen(self.game.window, self.game.map).open_()

    def check_garrison_existence(self):
        if self.garrison.empty():
            self.garrison = None

    def accept_visitor(self, squad):
        if (self.master == self.game.map.antagonist
                and self.garrison is not None):

            squad.fight(self.garrison)
            self.check_garrison_existence()
            self.game.map.clear_squads()
            if squad.empty():
                return

        self.change_master(squad.player)

        if self.garrison is None:
            self.game.map.add_squad(self.game.map.protagonist, -1, -1)
            self.garrison = self.game.map.squads[-1]

        self.guest = squad
        self.open_fortress_menu()

    def throw_guest_away(self):
        self.check_garrison_existence()
        if self.guest.empty():
            self.game.map.selected_squad = self.garrison
        else:
            self.game.map.selected_squad = self.guest
            self.game.map.move_selected_squad(0, -1)

        self.guest = None
        self.game.map.clear_squads()
        self.close_fortress_menu()

        self.game.map.send_state()

        if not self.game.map.check_game_end():
            self.game.map.check_turn_end()
        self.game.window.redraw()


class Map(INetworkEventSubscriber, IMapObjInfoConvertible):
    WIDTH = 32
    HEIGHT = 32
    FORTRESSES_NUMBER = 12
    ONE_TURN_MOVES = 10

    def __init__(self, game):
        self.game = game
        self.squads = []
        self.fortresses = []
        self.protagonist = Player(self)
        self.antagonist = Player(self)
        self.turn = None
        self.selected_squad = None
        self.moves_left = 0

        self.game.network.subscribe('connect', self)
        self.game.network.subscribe('map_update', self)

    def get_info(self):
        turn = self.get_player_code(self.turn)

        selected_squad = self.get_index_of_squad(self.selected_squad)

        info = {'squads': [],
                'fortresses': [],
                'selected_squad': selected_squad,
                'moves_left': self.moves_left,
                'turn': turn,
                'protagonist': self.protagonist.get_info(),
                'antagonist': self.antagonist.get_info()}

        for i in self.squads:
            info['squads'].append(i.get_info())

        for i in self.fortresses:
            info['fortresses'].append(i.get_info())

        return info

    def reset_from_info(self, info):
        self.protagonist = Player(self)
        self.antagonist = Player(self)

        self.protagonist.reset_from_info(info['antagonist'])
        self.antagonist.reset_from_info(info['protagonist'])

        self.squads = []
        self.fortresses = []

        for i in info['squads']:
            player = self.get_player_by_code(i['player'])
            self.squads.append(Squad(player))
            self.squads[-1].reset_from_info(i)

        for i in info['fortresses']:
            self.fortresses.append(Fortress(self.game, i['x'], i['y']))
            self.fortresses[-1].reset_from_info(i)

        self.moves_left = info['moves_left']
        self.turn = self.get_player_by_code(info['turn'])
        self.selected_squad = self.get_squad_by_index(info['selected_squad'])

    def get_index_of_squad(self, squad):
        try:
            return self.squads.index(squad)
        except ValueError:
            return -1

    def get_squad_by_index(self, index):
        if index == -1:
            return None
        return self.squads[index]

    def get_player_code(self, player):
        if player == self.antagonist:
            return 0
        elif player == self.protagonist:
            return 1
        return -1

    def get_player_by_code(self, code):
        if code == -1:
            return None
        return self.protagonist if code == 0 else self.antagonist

    def handle_network_event(self, type_, event):
        if type_ == 'connect':
            self.on_network_connected(event['host'])
        elif type_ == 'map_update':
            self.reset_from_info(event)
            if not self.check_game_end():
                self.check_turn_end()
        self.game.window.redraw()

    def on_network_connected(self, is_host):
        if is_host:
            self.generate_map()
            self.send_state()

    def get_fort_by_garrison(self, squad):
        return [i for i in self.fortresses if i.garrison is squad][0]

    def move_selected_squad(self, dx, dy):
        if self.selected_squad.is_garrison():
            fort = self.get_fort_by_garrison(self.selected_squad)
            self.add_squad(self.protagonist, fort.x, fort.y)
            fort.guest = self.squads[-1]
            fort.open_fortress_menu()
            return

        x = self.selected_squad.x
        y = self.selected_squad.y
        x += dx
        y += dy
        if not self.is_valid_cell(x, y):
            return
        self.selected_squad.move(dx, dy)

        squads_at_cell = self.squads_at_cell(x, y)
        fort_at_cell = self.forts_at_cell(x, y)

        if fort_at_cell:
            fort_at_cell[0].accept_visitor(self.selected_squad)
            self.moves_left += 1
        elif len(squads_at_cell) == 2:
            other_squad = (squads_at_cell[0] if squads_at_cell[1]
                           is self.selected_squad else squads_at_cell[1])
            self.selected_squad.interact(other_squad)
            self.clear_squads()

        self.moves_left -= 1
        self.send_state()

        if not self.check_game_end():
            self.check_turn_end()

    def end_game(self, result):
        screens.EndScreen(self.game.window, result).open_()

    def check_game_end(self):
        protagonist_forts =\
            [i for i in self.fortresses if i.master == self.protagonist]
        antagonist_forts =\
            [i for i in self.fortresses if i.master == self.antagonist]
        protagonist_squads =\
            [i for i in self.squads if i.player == self.protagonist and not i.empty()]
        antagonist_squads =\
            [i for i in self.squads if i.player == self.antagonist and not i.empty()]

        if len(protagonist_squads) == 0 and len(antagonist_squads) == 0:
            self.end_game(-1)
            return True
        elif len(protagonist_squads) == 0:
            self.end_game(1)
            return True
        elif len(antagonist_squads) == 0:
            self.end_game(0)
            return True
        elif len(protagonist_forts) == self.FORTRESSES_NUMBER:
            self.end_game(0)
            return True
        elif len(antagonist_forts) == self.FORTRESSES_NUMBER:
            self.end_game(1)
            return True
        return False

    def check_turn_end(self):
        if not self.moves_left:
            self.start_turn(self.protagonist
                            if self.turn == self.antagonist
                            else self.antagonist)

    def add_squad(self, player, x, y):
        self.squads.append(Squad(player, x, y))

    def clear_squads(self):
        while self.selected_squad.empty():
            self.select_other_squad()
        self.squads = [squad for squad in self.squads if not squad.empty()]

    def select_first_squad(self, player):
        index = 0
        while self.squads[index].player != player:
            index += 1
        self.selected_squad = self.squads[index]

    def select_other_squad(self, delta=1):
        index = (self.squads.index(self.selected_squad) + delta) % len(self.squads)
        while self.squads[index].player != self.protagonist:
            index = (index + delta) % len(self.squads)
        self.selected_squad = self.squads[index]

    def send_state(self):
        state = self.get_info()
        self.game.network.send_message({'type': 'map_update', 'event': state})

    def squads_at_cell(self, x, y):
        return [squad for squad in self.squads
                if squad.x == x and squad.y == y]

    def forts_at_cell(self, x, y):
        return [fort for fort in self.fortresses
                if fort.x == x and fort.y == y]

    def is_valid_cell(self, x, y):
        return x in range(self.WIDTH) and y in range(self.HEIGHT)

    def is_empty_cell(self, x, y):
        return (self.is_valid_cell(x, y) and
                not self.squads_at_cell(x, y) and
                not self.forts_at_cell(x, y))

    def start_turn(self, player):
        self.moves_left = self.ONE_TURN_MOVES
        self.turn = player

        for i in self.fortresses:
            if i.master == player:
                player.gold += Fortress.FORTRESS_PROFIT

        self.select_first_squad(player)

    def generate_map(self):
        self.add_squad(self.protagonist, 0, 0)
        self.squads[-1].add_soldier(Soldier(self.squads[-1]))

        self.add_squad(self.antagonist, self.WIDTH - 1, self.HEIGHT - 1)
        self.squads[-1].add_soldier(Soldier(self.squads[-1]))

        for i in range(self.FORTRESSES_NUMBER):
            fort_x, fort_y = -1, -1
            while True:
                fort_x = random.randrange(self.WIDTH)
                fort_y = random.randrange(self.HEIGHT)
                if (self.is_empty_cell(fort_x, fort_y) and
                        self.is_empty_cell(fort_x, fort_y - 1)):
                    break

            self.fortresses.append(Fortress(self.game, fort_x, fort_y))

        self.start_turn(self.protagonist if random.randrange(2)
                        else self.antagonist)


class Soldier(IMapObjInfoConvertible):
    DEFAULT_COST = 30
    DEFAULT_ARMOR = 0
    DEFAULT_ATTACK = 30
    BONUS_COST = 100
    MIN_ATTACK = 30
    MAX_ATTACK = 230
    MIN_ARMOR = 0
    MAX_ARMOR = 80

    def __init__(self, squad=None, armor=DEFAULT_ARMOR, attack=DEFAULT_ATTACK):
        self.squad = squad
        self.armor = armor
        self.attack = attack
        self.hp = 100

    def get_info(self):
        info = {'armor': self.armor,
                'attack': self.attack,
                'hp': self.hp
                }
        return info

    def reset_from_info(self, info):
        self.armor = info['armor']
        self.attack = info['attack']
        self.hp = info['hp']

    def alive(self) -> bool:
        return self.hp > 0

    def fight(self, enemy):
        my_attack = self.attack * (100 - enemy.armor) // 100
        enemy_attack = enemy.attack * (100 - self.armor) // 100
        self.hp -= enemy_attack
        enemy.hp -= my_attack

    def cost(self) -> int:
        delta_armor = self.armor - self.MIN_ARMOR
        delta_attack = self.attack - self.MIN_ATTACK
        delta_armor_percent = delta_armor / (self.MAX_ARMOR - self.MIN_ARMOR)
        delta_attack_percent = delta_attack / (self.MAX_ATTACK - self.MIN_ATTACK)
        return (
                self.DEFAULT_COST +
                int(self.BONUS_COST * (delta_armor_percent + delta_attack_percent))
        )


class Squad(IMapObjInfoConvertible):
    def __init__(self, player, x=-1, y=-1):
        self.player = player
        self.x = x
        self.y = y
        self.soldiers = []

    def get_info(self):
        player = self.player.map.get_player_code(self.player)
        info = {'player': player,
                'x': self.x,
                'y': self.y,
                'soldiers': []}

        for i in self.soldiers:
            info['soldiers'].append(i.get_info())

        return info

    def __len__(self):
        return len(self.soldiers)

    def reset_from_info(self, info):
        self.x = info['x']
        self.y = info['y']
        self.soldiers = []

        for i in info['soldiers']:
            self.soldiers.append(Soldier(self))
            self.soldiers[-1].reset_from_info(i)

    def move(self, dx, dy):
        self.x += dx
        self.y += dy

    def add_soldier(self, soldier: Soldier):
        self.soldiers.append(soldier)

    def empty(self) -> bool:
        return len(self.soldiers) == 0

    def update(self):
        self.soldiers = [soldier for soldier in self.soldiers if soldier.alive()]

    def size(self) -> int:
        return len(self.soldiers)

    def is_garrison(self) -> bool:
        return self.x == -1

    def fight(self, enemy) -> bool:
        while (not self.empty()) and (not enemy.empty()):
            for i in range(min(self.size(), enemy.size())):
                self.soldiers[i].fight(enemy.soldiers[i])
            self.update()
            enemy.update()
        if self.empty():
            # Defeat
            return False
        else:
            # Victory
            return True

    def unite(self, friend_squad):
        self.soldiers += friend_squad.soldiers
        friend_squad.soldiers = []

    def interact(self, other):
        if self.player == other.player:
            self.unite(other)
        else:
            self.fight(other)
