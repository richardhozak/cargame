extends Node


var _tasks: Array[Callable]


func _ready() -> void:
	process_physics_priority = 1


func _physics_process(_delta: float) -> void:
	if _tasks.size() == 0:
		return

	var group_id := WorkerThreadPool.add_group_task(func(i: int): _tasks[i].call(), _tasks.size(), -1, true, "Car Physics Simulation")
	WorkerThreadPool.wait_for_group_task_completion(group_id)
	_tasks.clear()


func queue_simulate(player: Player) -> void:
	var simulate_player := func() -> void:
		for input in player.driver.get_next_inputs():
			player.simulate(input)

	_tasks.push_back(simulate_player)
