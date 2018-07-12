#!/bin/bash

_CAD=$(cd $(dirname $0); pwd)                      # 当前脚本目录
cd $_CAD
#host_dir=`pwd`                                    # 当前用户根目录(废弃使用)
proc_name="autohdmap_compile_server"                              # 进程名，根据引擎的进程名进行修改
conf_file="autohdmap_compile_server.cfg"
start_param=
full_proc=${_CAD}/${proc_name}                     # 进程名全路径
data_param=Def_Path
port_param=Def_Port
pid=0
chmod +x ${full_proc}

log_file=${_CAD}/${proc_name}.log                       # 日志文件，默认记录引擎目录下 
m_log_file=${_CAD}/${proc_name}-monitor.log             # 启动记录日志文件，默认记录在引擎目录下

monitor_interval=10

########  引擎配置文件  ##########
engine_config=$_CAD/${conf_file}

########  msp主配置文件开始  #########
# 如果部署包conf/gis.conf文件存在，则使之生效
msp_config=${_CAD}/../../conf/gis.conf
if [ -f $msp_config ];then
        source $msp_config
########  端口设置  ############
# 如果部署包conf/gis.conf有配置，使用这里的配置；如果没有，则使用引擎包的配置文件的端口
        gism=`echo ${proc_name}_port` && gismm=\$${gism} && engine_port=`eval echo $gismm`
        #echo $engine_port
        if [ ! -z $engine_port ];then  # 如果主配置文件中的端口有设置，则替换引擎目录中的配置文件
                sed -i "s#\($port_param\).*#${port_param}=$engine_port#g" $engine_config
        fi

#######  数据设置  ############
# 如果部署包data/engine/$proc_name 目录下有数据文件，则使用那里的数据，如果没有，则使用引擎目录下面的
        gism=`echo ${proc_name}_data` && gismm=\$${gism} && engine_data=`eval echo $gismm` # 取主配置文件中的数据目录配置
        # if [ ! -z ${engine_data} -a -d ${engine_data} ];then   # 如果主配置文件中的数据文件有设置且目录存在
        if [ ! -z ${engine_data} ]; then        # 只要主配置文件中有设置，而不为空即替换
                #if [ ! -z "`ls $engine_data`" ];then  # 如果该数据目录中的非空
                sed -i "s#\(${data_param}=\).*#${data_param}=${engine_data}#g" $engine_config
                #fi
        fi

######## 日志文件设置  ##########
# 如果部署包logs/engine/日志目录存在，则统一存储在这里, 否则保存在当前引擎目录下
        if [ ! -z ${logs_dir} -a -d ${logs_dir} ]; then  # 主配置文件中的日志目录存在
                if [ ! -d ${logs_dir}/engine_log ];then
                        mkdir ${logs_dir}/engine_log
                fi
                log_file=${logs_dir}/engine_log/${proc_name}.log
                m_log_file=${logs_dir}/engine_log/${proc_name}-monitor.log
        fi

######## 重启脚本检查时间间隔  #####
        if [ ! -z $check_interval ];then # 如果主配置文件中的check_interval存在，则替换本shell中的
                monitor_interval=${check_interval}
        fi
fi

proc_num() {
        num=`ps -ef | grep -w ${full_proc} | grep -v grep | wc -l`
        return $num
}
proc_id() {
        pid=`ps -ef | grep -w ${full_proc} | grep -v grep | awk '{print $2}'`
}

start() {
        proc_num
        if [ $? -eq 0 ]; then
                nohup ${full_proc} ${start_param} >> ${log_file} 2>&1 &
                sleep 0.1
                proc_id
                dp=`echo ${port_param}`&&dpp=\$$dp&&dppp=`eval echo $dpp`
                echo $proc_name pid=${pid} port=${dppp} start success  [`date "+%F %T"`] >> ${m_log_file}     
                echo $proc_name pid=${pid} port=${dppp} start success. [`date "+%F %T"`]
        else
                proc_id
                echo $proc_name pid=${pid} alreay started.
        fi 
}
stop(){
        proc_num
        if [ $? -eq 0 ]; then         
                echo $proc_name not started.
        else
                proc_id
                kill -9 ${pid}
                echo $proc_name pid=${pid} has been shutdown. [`date "+%F %T"`]
        fi
}

status() {
        ps -ef|grep -w ${full_proc} |grep -v grep
}
info(){
        tail -fn200 ${log_file}
}
monitor(){
        while [ 1 ]; do
                proc_num
                number=$?
                if [ $number -eq 0 ]; then
                        start
                        echo "$proc_name start at `date "+%F %T"`" automatically. >> $m_log_file
                fi
                sleep $monitor_interval
        done
}

case "$1" in
        start) start;;
        stop) stop;;
        restart) stop;sleep .2;start;;
		status) status;;
        monitor) monitor;;
        info) info;;
        *) echo "usage: $0 start|stop|restart|status|info";;
esac
