//后期改进思路：加入两个权重概念(任务完成时间+对后车的阻滞情况)
//还得加入一个模块：调控前车/后车的加速度（加在哪里还没想好）

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

//真正把任务分配给车，并生成执行计划。
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

procedure assignTaskToCar(task, car, current_time){

    t_start ← max(car.available_time, task.ready_time)
    d1 ← calcDistance(car.position, task.start_device)
    t1 ← calcTravelTime(d1)

    d2 ← calcDistance(task.start_device, task.end_device)
    t2 ← calcTravelTime(d2)

    t_arrive_pick ← t_start + t1
    t_pick_done ← t_arrive_pick + LoadTime
    t_arrive_drop ← t_pick_done + t2
    t_drop_done ← t_arrive_drop + UnloadTime

    // 更新车辆状态
    car.position ← task.end_device
    car.available_time ← t_drop_done
    car.is_loaded ← false
    car.is_executing ← true
    car.task_log.append(task.id)

    // 更新任务执行记录
    task.assign_time ← current_time
    task.pick_time ← t_arrive_pick
    task.drop_time ← t_arrive_drop
    task.complete_time ← t_drop_done
    task.car_id ← car.id
    task.is_assigned ← true

    // 插入到日志缓冲队列中，稍后统一写入
    pushToTaskExeLogBuffer(task)}

//防止设备并发冲突，提前锁定设备资源。
procedure reserveDeviceForTask(task, current_time){

    start ← device_states[task.start_device]
    end ← device_states[task.end_device]

    // 任务执行阶段持续时间估算
    d1 ← calcDistance(task.assigned_car_position, task.start_device)
    t1 ← calcTravelTime(d1)

    d2 ← calcDistance(task.start_device, task.end_device)
    t2 ← calcTravelTime(d2)

    t_start ← max(task.ready_time, task.assign_time)
    t_arrive_pick ← t_start + t1
    t_pick_done ← t_arrive_pick + LoadTime
    t_arrive_drop ← t_pick_done + t2
    t_drop_done ← t_arrive_drop + UnloadTime

    // 标记起点设备：正在等待小车取货（只锁定到装完货）
    start.is_reserved ← true
    start.reserved_by ← task.id
    start.reserved_until ← t_pick_done

    // 标记终点设备：正在等待小车送货（锁定到卸货完成）
    end.is_reserved ← true
    end.reserved_by ← task.id
    end.reserved_until ← t_drop_done
}
// 作用：在每次仿真推进时更新车辆执行状态
procedure updateCarStatus(current_time){

    for car in all_cars:

        if car.is_executing and current_time ≥ car.available_time:

            // 表示任务已完成
            car.is_executing ← false
            car.is_loaded ← false

            // 更新位置（已在 assignTaskToCar 中更新）
            // 可选择将当前位置广播到轨道状态中，释放占位

            // 更新车辆轨迹日志（用于轨道调度/可视化）
            recordCarFree(car.id, car.position, current_time)

            // 同时释放相关设备的状态锁
            last_task ← getLastTaskByCar(car.id)

            device_states[last_task.start_device].is_reserved ← false
            device_states[last_task.start_device].reserved_by ← null

            device_states[last_task.end_device].is_reserved ← false
            device_states[last_task.end_device].reserved_by ← null

            // 更新目标设备状态：有货 or 空 depending on task type
            if last_task.type == 出库:
                device_states[last_task.end_device].has_goods ← true
                device_states[last_task.end_device].awaiting_unload ← true  // 等人搬货
                scheduleHumanUnloading(last_task, current_time)

            else if last_task.type == 入库:
                device_states[last_task.end_device].has_goods ← true
                device_states[last_task.end_device].awaiting_pick ← true    // 等堆垛机
                scheduleStackerPick(last_task, current_time)
}