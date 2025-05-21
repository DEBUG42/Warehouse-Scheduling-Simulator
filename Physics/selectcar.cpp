function filterAvailableCars(task, all_cars, current_time):

    candidates ← []

    for car in all_cars:
        if car.is_loaded:
            continue  // 正在搬运任务

        t_ready ← max(car.available_time, task.ready_time)

        distance_to_start ← calcDistance(car.position, task.start_device)
        t_travel ← calcTravelTime(distance_to_start)

        t_arrive ← t_ready + t_travel

        if not isDeviceBusy(task.start_device, t_arrive):
            if not willCauseCollision(car, task.start_device, t_arrive):
                candidates.append(car)

    return candidates



function selectCarWithEarliestFinish(task, candidate_cars, current_time):

    best_car ← null
    min_finish_time ← ∞

    for car in candidate_cars:

        t_ready ← max(car.available_time, task.ready_time)

        d1 ← calcDistance(car.position, task.start_device)
        t1 ← calcTravelTime(d1)

        d2 ← calcDistance(task.start_device, task.end_device)
        t2 ← calcTravelTime(d2)

        t_total ← t_ready + t1 + LoadTime + t2 + UnloadTime

        if t_total < min_finish_time:
            min_finish_time ← t_total
            best_car ← car

    return best_car
