function filterAvailableCars(task, all_cars, current_time){

    candidates ← []

    for car in all_cars:

        // Step 1: 排除非空闲车辆（正在执行任务）
        if car.is_loaded or car.is_executing:
            continue

        // Step 2: 预计出发时刻（车辆可用时间或任务准备时间中的较晚者）
        t_ready ← max(car.available_time, task.ready_time)

        // Step 3: 计算车辆从当前位置到任务起点所需时间
        distance_to_start ← calcDistance(car.position, task.start_device)
        t_travel ← calcTravelTime(distance_to_start)
        t_arrive ← t_ready + t_travel

        // Step 4: 判断任务起点设备是否在预计到达时间可取货
        if isDeviceBusy(task.start_device, t_arrive):
            continue

        // Step 5: 碰撞预测 —— 计划路径上是否会与其他车辆冲突
        if willCauseCollision(car, task.start_device, t_arrive):
            continue

        // Step 6: 若无异常，加入候选车列表
        candidates.append(car)

    return candidates
}

function selectCarWithEarliestFinish(task, candidate_cars, current_time){

    best_car ← null
    min_finish_time ← ∞

    for car in candidate_cars:

        // Step 1: 任务执行起始时刻 = max(车辆可用时间, 任务准备时间)
        t_start ← max(car.available_time, task.ready_time)

        // Step 2: 起点与终点的行驶距离与耗时
        d1 ← calcDistance(car.position, task.start_device)
        t1 ← calcTravelTime(d1)

        d2 ← calcDistance(task.start_device, task.end_device)
        t2 ← calcTravelTime(d2)

        // Step 3: 完整任务总耗时 = 等待 + 去起点 + 装货 + 去终点 + 卸货
        t_finish ← t_start + t1 + LoadTime + t2 + UnloadTime

        // Step 4: 比较，保留最早完成任务的车辆
        if t_finish < min_finish_time:
            min_finish_time ← t_finish
            best_car ← car

    return best_car
}