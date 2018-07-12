/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: weihainan
 *
 * Created on 2017年10月20日, 上午11:40
 */

#include "AutomapDataCheckServer.h"
#include "AutomapDataCheckTask.h"

/*
 * 
 */
int main(int argc, char **argv) {

    AutomapDataCheckTask *pTask = new AutomapDataCheckTask();
    AutomapDataCheckServer srv;
    return srv.startServer(pTask, argc, argv);
}

