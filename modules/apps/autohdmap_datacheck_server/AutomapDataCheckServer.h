/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   AutoMapServer.h
 * Author: zhangbenxing
 *
 * Created on 2017年10月20日, 上午11:52
 */

#ifndef AutomapDataCheckServer_H
#define AutomapDataCheckServer_H

#include "KdServer.h"

class AutomapDataCheckServer : public KdServer {
public:
    AutomapDataCheckServer();

    AutomapDataCheckServer(const AutomapDataCheckServer &orig);

    ~AutomapDataCheckServer() override;


private:

};

#endif /* AUTOMAPSERVER_H */

