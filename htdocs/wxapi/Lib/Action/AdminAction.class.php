<?php
/*************************************************************
    HaoYi (C)2017 - 2020 myhaoyi.com

*************************************************************/

class AdminAction extends Action
{
  public function _initialize() {
  }
  /**
  +----------------------------------------------------------
  * 默认操作...
  +----------------------------------------------------------
  */
  public function index()
  {
    // 重定向到默认的系统页面...
    header("location:".__APP__.'/Admin/system');
  }
  //
  // 响应点击测试连接 => Tracker...
  public function testTracker()
  {
    $tracker = fastdfs_connect_server($_POST['addr'], $_POST['port']);
    if( $tracker ) {
      fastdfs_disconnect_server($tracker);
    }
    echo (is_array($tracker) ? 1 : 0);
  }
  //
  // 响应点击测试连接 => Transmit...
  public function testTransmit()
  {
    $transmit = transmit_connect_server($_POST['addr'], $_POST['port']);
    if( $transmit ) {
      transmit_disconnect_server($transmit);
    }
    echo (is_array($transmit) ? 1 : 0);
  }
  //
  // 获取系统配置页面...
  public function system()
  {
    $this->assign('my_title', "云录播 - 网站管理");
    $this->assign('my_command', 'system');

    // 查找系统设置记录...
    $dbSys = D('system')->find();
    // 获取传递过来的参数...
    $theOperate = $_POST['operate'];
    if( $theOperate == 'save' ) {
      // 更新数据库记录，直接存POST数据...
      $_POST['system_id'] = $dbSys['system_id'];
      D('system')->save($_POST);
    } else {      
      // 设置模板参数，并返回数据...
      $this->assign('my_sys', $dbSys);
      $this->display();
    }
  }
  //
  // 获取科目页面...
  public function subject()
  {
    $this->assign('my_title', "云录播 - 科目管理");
    $this->assign('my_command', 'subject');

    // 得到每页条数，总记录数，计算总页数...
    $pagePer = C('PAGE_PER');
    $totalNum = D('subject')->count();
    $max_page = intval($totalNum / $pagePer);
    // 判断是否是整数倍的页码...
    $max_page += (($totalNum % $pagePer) ? 1 : 0);

    // 设置最大页数，设置模板参数...
    $this->assign('max_page', $max_page);
    $this->display();
  }
  //
  // 获取科目分页数据...
  public function pageSubject()
  {
    // 准备需要的分页参数...
    $pagePer = C('PAGE_PER'); // 每页显示的条数...
    $pageCur = (isset($_GET['p']) ? $_GET['p'] : 1);  // 当前页码...
    $pageLimit = (($pageCur-1)*$pagePer).','.$pagePer; // 读取范围...
    
    // 查询分页数据，设置模板...
    $arrSubject = D('subject')->limit($pageLimit)->order('subject_id DESC')->select();
    $this->assign('my_subject', $arrSubject);
    echo $this->fetch('pageSubject');
  }
  //
  // 保存科目数据...
  public function saveSubject()
  {
    if( $_POST['subject_id'] <= 0 ) {
      unset($_POST['subject_id']);
      $_POST['created'] = date('Y-m-d H:i:s');
      $_POST['updated'] = date('Y-m-d H:i:s');
      D('subject')->add($_POST);
    } else {
      $_POST['updated'] = date('Y-m-d H:i:s');
      D('subject')->save($_POST);
    }
  }
  //
  // 删除科目数据...
  public function delSubject()
  {
    D('subject')->where('subject_id='.$_POST['subject_id'])->delete();
  }
  //
  // 获取年级页面...
  public function grade()
  {
    $this->assign('my_title', "云录播 - 年级管理");
    $this->assign('my_command', 'grade');
    // 得到每页条数，总记录数，计算总页数...
    $pagePer = C('PAGE_PER');
    $totalNum = D('grade')->count();
    $max_page = intval($totalNum / $pagePer);
    // 判断是否是整数倍的页码...
    $max_page += (($totalNum % $pagePer) ? 1 : 0);
    // 设置最大页数，设置模板参数...
    $this->assign('max_page', $max_page);
    $this->display();
  }
  //
  // 获取年级分页数据...
  public function pageGrade()
  {
    // 准备需要的分页参数...
    $pagePer = C('PAGE_PER'); // 每页显示的条数...
    $pageCur = (isset($_GET['p']) ? $_GET['p'] : 1);  // 当前页码...
    $pageLimit = (($pageCur-1)*$pagePer).','.$pagePer; // 读取范围...
    
    // 查询分页数据，设置模板...
    $arrGrade = D('grade')->limit($pageLimit)->order('grade_id DESC')->select();
    $this->assign('my_grade', $arrGrade);
    echo $this->fetch('pageGrade');
  }
  //
  // 保存年级数据...
  public function saveGrade()
  {
    if( $_POST['grade_id'] <= 0 ) {
      unset($_POST['grade_id']);
      $_POST['created'] = date('Y-m-d H:i:s');
      $_POST['updated'] = date('Y-m-d H:i:s');
      D('grade')->add($_POST);
    } else {
      $_POST['updated'] = date('Y-m-d H:i:s');
      D('grade')->save($_POST);
    }
  }
  //
  // 删除年级数据...
  public function delGrade()
  {
    D('grade')->where('grade_id='.$_POST['grade_id'])->delete();
  }
  //
  // 获取教师页面...
  public function teacher()
  {
    $this->assign('my_title', "云录播 - 教师管理");
    $this->assign('my_command', 'teacher');
    // 得到每页条数，总记录数，计算总页数...
    $pagePer = C('PAGE_PER');
    $totalNum = D('teacher')->count();
    $max_page = intval($totalNum / $pagePer);
    // 判断是否是整数倍的页码...
    $max_page += (($totalNum % $pagePer) ? 1 : 0);
    // 设置最大页数，设置模板参数...
    $this->assign('max_page', $max_page);
    $this->display();
  }
  //
  // 获取教师分页数据...
  public function pageTeacher()
  {
    // 准备需要的分页参数...
    $pagePer = C('PAGE_PER'); // 每页显示的条数...
    $pageCur = (isset($_GET['p']) ? $_GET['p'] : 1);  // 当前页码...
    $pageLimit = (($pageCur-1)*$pagePer).','.$pagePer; // 读取范围...
    
    // 查询分页数据，设置模板...
    $arrTeacher = D('teacher')->limit($pageLimit)->order('teacher_id DESC')->select();
    $this->assign('my_teacher', $arrTeacher);
    echo $this->fetch('pageTeacher');
  }
  //
  // 保存教师数据...
  public function saveTeacher()
  {
    if( $_POST['teacher_id'] <= 0 ) {
      unset($_POST['teacher_id']);
      $_POST['created'] = date('Y-m-d H:i:s');
      $_POST['updated'] = date('Y-m-d H:i:s');
      D('teacher')->add($_POST);
    } else {
      $_POST['updated'] = date('Y-m-d H:i:s');
      D('teacher')->save($_POST);
    }
  }
  //
  // 删除教师数据...
  public function delTeacher()
  {
    D('teacher')->where('teacher_id='.$_POST['teacher_id'])->delete();
  }
  //
  // 获取教师修改页面 => 这是一个完整页面，因为ajax模式无法加载layui...
  public function getTeacher()
  {
    $this->assign('my_title', "云录播 - 教师管理");
    $this->assign('my_command', 'teacher');
    $theID = $_GET['teacher_id'];
    if( $theID > 0 ) {
      $dbTeacher = D('teacher')->where('teacher_id='.$theID)->find();
      if( $dbTeacher['title_id'] <= 0 ) {
        $dbTeacher['title_id'] = 0;
      }
      $this->assign('my_teacher', $dbTeacher);
      $this->assign('my_new_title', "修改 - 教师");
    } else {
      $dbTeacher['teacher_id'] = 0;
      $dbTeacher['sex_name'] = '男';
      $dbTeacher['title_id'] = 2;
      $this->assign('my_teacher', $dbTeacher);
      $this->assign('my_new_title', "添加 - 教师");
    }
    $this->display();
  }
  //
  // 获取学校页面...
  public function school()
  {
    $this->assign('my_title', "云录播 - 学校管理");
    $this->assign('my_command', 'school');
    // 得到每页条数，总记录数，计算总页数...
    $pagePer = C('PAGE_PER');
    $totalNum = D('school')->count();
    $max_page = intval($totalNum / $pagePer);
    // 判断是否是整数倍的页码...
    $max_page += (($totalNum % $pagePer) ? 1 : 0);
    // 设置最大页数，设置模板参数...
    $this->assign('max_page', $max_page);
    $this->display();
  }
  //
  // 获取学校分页数据...
  public function pageSchool()
  {
    // 准备需要的分页参数...
    $pagePer = C('PAGE_PER'); // 每页显示的条数...
    $pageCur = (isset($_GET['p']) ? $_GET['p'] : 1);  // 当前页码...
    $pageLimit = (($pageCur-1)*$pagePer).','.$pagePer; // 读取范围...
    
    // 查询分页数据，设置模板...
    $arrSchool = D('school')->limit($pageLimit)->order('school_id DESC')->select();
    $this->assign('my_school', $arrSchool);
    echo $this->fetch('pageSchool');
  }
  //
  // 获取学校修改页面 => 这是一个完整页面，因为ajax模式无法加载layui...
  public function getSchool()
  {
    $this->assign('my_title', "云录播 - 学校管理");
    $this->assign('my_command', 'school');
    $theID = $_GET['school_id'];
    if( $theID > 0 ) {
      $dbSchool = D('school')->where('school_id='.$theID)->find();
      $this->assign('my_school', $dbSchool);
      $this->assign('my_new_title', "修改 - 学校");
    } else {
      $dbSchool['school_id'] = 0;
      $this->assign('my_school', $dbSchool);
      $this->assign('my_new_title', "添加 - 学校");
    }
    $this->display();
  }
  //
  // 保存学校操作...
  public function saveSchool()
  {
    if( $_POST['school_id'] <= 0 ) {
      unset($_POST['school_id']);
      $_POST['created'] = date('Y-m-d H:i:s');
      $_POST['updated'] = date('Y-m-d H:i:s');
      echo D('school')->add($_POST);
    } else {
      $_POST['updated'] = date('Y-m-d H:i:s');
      D('school')->save($_POST);
      echo $_POST['school_id'];
    }
  }
  //
  // 删除教师数据...
  public function delSchool()
  {
    D('school')->where('school_id='.$_POST['school_id'])->delete();
  }
  //
  // 获取采集端页面...
  public function gather()
  {
    $this->assign('my_title', "云录播 - 采集端管理");
    $this->assign('my_command', 'gather');
    // 得到每页条数，总记录数，计算总页数...
    $pagePer = C('PAGE_PER');
    $totalNum = D('gather')->count();
    $max_page = intval($totalNum / $pagePer);
    // 判断是否是整数倍的页码...
    $max_page += (($totalNum % $pagePer) ? 1 : 0);
    // 设置最大页数，设置模板参数...
    $this->assign('max_page', $max_page);
    $this->display();
  }
  //
  // 获取采集端在线状态标志...
  private function getGatherStatusFromTransmit(&$arrGather)
  {
    // 尝试链接中转服务器...
    $dbSys = D('system')->field('transmit_addr,transmit_port')->find();
    $transmit = transmit_connect_server($dbSys['transmit_addr'], $dbSys['transmit_port']);
    foreach($arrGather as &$dbItem) {
      // 设置采集端默认状态...
      $dbItem['status'] = 0;
      // 获取采集端在线状态...
      if( $transmit ) {
        $dbJson['mac_addr'] = $dbItem['mac_addr'];
        $saveJson = json_encode($dbJson);
        $json_data = transmit_command(kClientPHP, kCmd_PHP_Get_Gather_Status, $transmit, $saveJson);
        if( $json_data ) {
          // 获取的JSON数据有效，转成数组，并判断错误码...
          $arrData = json_decode($json_data, true);
          if( $arrData['err_code'] == ERR_OK ) {
            $dbItem['status'] = $arrData['err_status'];
          }
        }
      }
    }
    // 关闭中转服务器链接...
    if( $transmit ) {
      transmit_disconnect_server($transmit);
    }
  }
  //
  // 获取采集端分页数据...
  public function pageGather()
  {
    // 准备需要的分页参数...
    $pagePer = C('PAGE_PER'); // 每页显示的条数...
    $pageCur = (isset($_GET['p']) ? $_GET['p'] : 1);  // 当前页码...
    $pageLimit = (($pageCur-1)*$pagePer).','.$pagePer; // 读取范围...
    // 查询分页数据，准备中转查询数据，设置默认状态...
    $arrGather = D('GatherView')->limit($pageLimit)->order('gather_id DESC')->select();
    // 获取采集端在线状态标志，设置模板参数...
    $this->getGatherStatusFromTransmit($arrGather);
    $this->assign('my_gather', $arrGather);
    echo $this->fetch('pageGather');
  }
  //
  // 获取采集端详情数据...
  public function getGather()
  {
    $this->assign('my_title', "云录播 - 采集端管理");
    $this->assign('my_command', 'gather');
    $theID = $_GET['gather_id'];
    if( $theID > 0 ) {
      $arrSchool = D('school')->field('school_id,name')->select();
      $dbGather = D('gather')->where('gather_id='.$theID)->find();
      $this->assign('my_gather', $dbGather);
      $this->assign('my_new_title', "修改 - 采集端");
      $this->assign('my_list_school', $arrSchool);
      $this->display();
    } else {
      echo '采集端不能通过网站添加！';
    }
  }
  //
  // 保存修改后的采集端数据...
  public function saveGather()
  {
    $_POST['updated'] = date('Y-m-d H:i:s');
    D('gather')->save($_POST);
    echo $_POST['gather_id'];
  }
  //
  // 获取采集端下面的摄像头列表...
  public function camera()
  {
    $this->assign('my_title', "云录播 - 摄像头");
    $this->assign('my_command', 'gather');
    // 得到每页条数，总记录数，计算总页数...
    $pagePer = C('PAGE_PER');
    $map['gather_id'] = $_GET['gather_id'];
    $totalNum = D('camera')->where($map)->count();
    $max_page = intval($totalNum / $pagePer);
    // 判断是否是整数倍的页码...
    $max_page += (($totalNum % $pagePer) ? 1 : 0);
    // 设置最大页数，设置模板参数...
    $this->assign('my_gather_id', $_GET['gather_id']);
    $this->assign('max_page', $max_page);
    $this->display();
  }
  //
  // 获取摄像头在线状态标志...
  private function getCameraStatusFromTransmit($strMacAddr, &$arrCamera)
  {
    // 尝试链接中转服务器...
    $dbSys = D('system')->field('transmit_addr,transmit_port')->find();
    $transmit = transmit_connect_server($dbSys['transmit_addr'], $dbSys['transmit_port']);
    // 先获取摄像头编号列表，并设置初始状态...
    foreach($arrCamera as &$dbItem) {
      $arrIDS[] = $dbItem['camera_id'];
      $dbItem['status'] = 0;
    }
    // 转换成JSON数据...
    $arrIDS["mac_addr"] = $strMacAddr;
    $saveJson = json_encode($arrIDS);
    // 查询指定摄像头列表的在线状态...
    if( $transmit ) {
      $json_data = transmit_command(kClientPHP, kCmd_PHP_Get_Camera_Status, $transmit, $saveJson);
      if( $json_data ) {
        // 获取的JSON数据有效，转成数组，并判断有没有错误码...
        $arrData = json_decode($json_data, true);
        if( is_array($arrData) && !isset($arrData['err_code']) ) {
          // 数组是有效的状态列表，将状态设置到对应的摄像头对象下面...
          foreach($arrCamera as &$dbItem) {
            foreach($arrData as $key => $value) {
              if( $dbItem['camera_id'] == $key ) {
                $dbItem['status'] = $value;
                break;
              }
            }
          }
        }
      }
    }
    // 关闭中转服务器链接...
    if( $transmit ) {
      transmit_disconnect_server($transmit);
    }
  }
  //
  // 获取摄像头分页数据...
  public function pageCamera()
  {
    // 准备需要的分页参数...
    $pagePer = C('PAGE_PER'); // 每页显示的条数...
    $pageCur = (isset($_GET['p']) ? $_GET['p'] : 1);  // 当前页码...
    $pageLimit = (($pageCur-1)*$pagePer).','.$pagePer; // 读取范围...
    // 获取采集端的MAC地址，以便查询状态...
    $map['gather_id'] = $_GET['gather_id'];
    $dbGather = D('gather')->where($map)->field('mac_addr')->find();
    // 设置查询条件，查询分页数据，设置模板...
    unset($map['gather_id']);
    $map['Camera.gather_id'] = $_GET['gather_id'];
    $arrCamera = D('CameraView')->where($map)->limit($pageLimit)->order('camera_id DESC')->select();
    // 获取摄像头在线状态标志...
    $this->getCameraStatusFromTransmit($dbGather['mac_addr'], $arrCamera);
    // 设置模板参数，返回模板数据...
    $this->assign('my_camera', $arrCamera);
    echo $this->fetch('pageCamera');
  }
  //
  // 获取摄像头详情数据...
  public function getCamera()
  {
    $this->assign('my_title', "云录播 - 摄像头");
    $this->assign('my_command', 'gather');
    // 获取传递过来的参数信息...
    $theCameraID = $_GET['camera_id'];
    if( $theCameraID > 0 ) {
      $arrGrade = D('grade')->field('grade_id,grade_type,grade_name')->order('grade_id ASC')->select();
      $map['camera_id'] = $theCameraID;
      $dbCamera = D('camera')->where($map)->find();
      $this->assign('my_camera', $dbCamera);
      $this->assign('my_new_title', "修改 - 摄像头");
      $this->assign('my_list_grade', $arrGrade);
      $this->display();
    } else {
      echo '摄像头不能通过网站添加！';
    }
  }
  //
  // 保存修改后的摄像头数据...
  public function saveCamera()
  {
    // 先将摄像头信息存入数据库当中...
    $_POST['updated'] = date('Y-m-d H:i:s');
    D('camera')->save($_POST);
    // 再将摄像头名称转发给对应的采集端...
    $dbSys = D('system')->field('transmit_addr,transmit_port')->find();
    $transmit = transmit_connect_server($dbSys['transmit_addr'], $dbSys['transmit_port']);
    // 转发设置摄像头名称命令...
    if( $transmit ) {
      // 获取转发节点的MAC地址...
      $map['gather_id'] = $_POST['gather_id'];
      $dbGather = D('gather')->where($map)->field('mac_addr')->find();
      // 组合摄像头数据成JSON...
      $dbCamera['camera_id'] = $_POST['camera_id'];
      $dbCamera['camera_name'] = sprintf("%s %s", $_POST['grade_name'], $_POST['camera_name']);
      $dbCamera['mac_addr'] = $dbGather['mac_addr'];
      $saveJson = json_encode($dbCamera);
      // 发送转发命令...
      $json_data = transmit_command(kClientPHP, kCmd_PHP_Set_Camera_Name, $transmit, $saveJson);
      // 关闭中转服务器链接...
      transmit_disconnect_server($transmit);
      // 反馈转发结果...
      echo $json_data;
    }
  }
  //
  // 获取摄像头(班级)下面的录像课程表...
  public function course()
  {
    $this->assign('my_title', "云录播 - 课程表");
    $this->assign('my_command', 'gather');
    // 获取传递过来的参数信息...
    $theCameraID = $_GET['camera_id'];
    $theGatherID = $_GET['gather_id'];
    // 得到每页条数，总记录数，计算总页数...
    $pagePer = C('PAGE_PER');
    $map['camera_id'] = $theCameraID;
    $totalNum = D('course')->where($map)->count();
    $max_page = intval($totalNum / $pagePer);
    // 判断是否是整数倍的页码...
    $max_page += (($totalNum % $pagePer) ? 1 : 0);
    // 获取班级年级信息...
    $dbGrade = D('CameraView')->where($map)->find();
    $this->assign('my_grade', $dbGrade);
    // 设置最大页数，设置模板参数...
    $this->assign('my_camera_id', $theCameraID);
    $this->assign('my_gather_id', $theGatherID);
    $this->assign('max_page', $max_page);
    $this->display();
  }
  //
  // 获取课程表分页数据...
  public function pageCourse()
  {
    // 准备需要的分页参数...
    $pagePer = C('PAGE_PER'); // 每页显示的条数...
    $pageCur = (isset($_GET['p']) ? $_GET['p'] : 1);  // 当前页码...
    $pageLimit = (($pageCur-1)*$pagePer).','.$pagePer; // 读取范围...
    // 获取传递过来的参数信息...
    $theCameraID = $_GET['camera_id'];
    $theGatherID = $_GET['gather_id'];
    // 获取采集端的MAC地址，以便查询状态...
    $map['gather_id'] = $theGatherID;
    $dbGather = D('gather')->where($map)->field('mac_addr')->find();
    // 设置查询条件，查询分页数据...
    unset($map['gather_id']);
    $map['Course.camera_id'] = $theCameraID;
    $arrCourse = D('CourseView')->where($map)->limit($pageLimit)->order('course_id DESC')->select();
    // 获取当前摄像头下正在录像的课表编号...
    $this->getCourseRecordFromTransmit($dbGather['mac_addr'], $theCameraID, $arrCourse);
    // 设置模板参数...
    $this->assign('my_course', $arrCourse);
    echo $this->fetch('pageCourse');
  }
  //
  // 获取摄像头下正在录像的课表编号...
  private function getCourseRecordFromTransmit($strMacAddr, $nCameraID, &$arrCourse)
  {
    // 尝试链接中转服务器...
    $dbSys = D('system')->field('transmit_addr,transmit_port')->find();
    $transmit = transmit_connect_server($dbSys['transmit_addr'], $dbSys['transmit_port']);
    // 查询指定摄像头列表的在线状态...
    if( $transmit ) {
      // 转换成JSON数据...
      $dbCamera['camera_id'] = $nCameraID;
      $dbCamera['mac_addr'] = $strMacAddr;
      $saveJson = json_encode($dbCamera);
      $json_data = transmit_command(kClientPHP, kCmd_PHP_Get_Course_Record, $transmit, $saveJson);
      if( $json_data ) {
        // 获取的JSON数据有效，转成数组...
        $arrData = json_decode($json_data, true);
        // 设置录像课表的状态 => course_id => 正在录像的编号...
        foreach($arrCourse as &$dbItem) {
          $dbItem['status'] = 0;
          if( (isset($arrData['course_id'])) && ($dbItem['course_id'] == $arrData['course_id']) ) {
            $dbItem['status'] = 1;
          }
        }
      }
    }
    // 关闭中转服务器链接...
    if( $transmit ) {
      transmit_disconnect_server($transmit);
    }
  }
  //
  // 获取课表修改页面 => 这是一个完整页面，因为ajax模式无法加载layui...
  public function getCourse()
  {
    $this->assign('my_title', "云录播 - 课程表");
    $this->assign('my_command', 'gather');
    $theCamera = $_GET['camera_id'];
    $theGather = $_GET['gather_id'];
    $theID = $_GET['course_id'];
    // 进行'修改'或'添加'分发...
    if( $theID > 0 ) {
      $dbCourse = D('course')->where('course_id='.$theID)->find();
      $this->assign('my_new_title', "修改 - 课程表");
    } else {
      $dbCourse['course_id'] = 0;
      $dbCourse['camera_id'] = $theCamera;
      $dbCourse['repeat_id'] = kNoneRepeat;
      $dbCourse['start_time'] = date('Y-m-d H:i:s');
      $dbCourse['end_time'] = date("Y-m-d H:i:s", strtotime("+1 hour"));;
      $this->assign('my_new_title', "添加 - 课程表");
    }
    // 读取科目列表，读取教师列表...
    $arrSubject = D('subject')->field('subject_id,subject_name')->select();
    $arrTeacher = D('teacher')->field('teacher_id,teacher_name,title_name')->select();
    // 添加记录时针对科目和教师的默认值...
    if( $theID <= 0 ) {
      $dbCourse['subject_id'] = (count($arrSubject) <= 0) ? 0 : $arrSubject[0]['subject_id'];
      $dbCourse['teacher_id'] = (count($arrTeacher) <= 0) ? 0 : $arrTeacher[0]['teacher_id'];
    }
    // 获取班级年级信息...
    $map['camera_id'] = $theCamera;
    $dbGrade = D('CameraView')->where($map)->find();
    $this->assign('my_grade', $dbGrade);  
    // 设置课程信息到模板...
    $dbCourse['gather_id'] = $theGather;
    $this->assign('my_course', $dbCourse);
    $this->assign('my_list_teacher', $arrTeacher);
    $this->assign('my_list_subject', $arrSubject);
    $this->display();
  }
  //
  // 保存课程表操作...
  public function saveCourse()
  {
    // 将传递的数据写入数据库当中...
    if( $_POST['course_id'] <= 0 ) {
      $nCmdType = kCmd_PHP_Set_Course_Add;
      unset($_POST['course_id']);
      $_POST['created'] = date('Y-m-d H:i:s');
      $_POST['updated'] = date('Y-m-d H:i:s');
      // 如果更新失败，直接返回...
      $result = D('course')->add($_POST);
      if( !$result ) return;
      // 更新成功记录course_id...
      $_POST['course_id'] = $result;
    } else {
      $nCmdType = kCmd_PHP_Set_Course_Mod;
      $_POST['updated'] = date('Y-m-d H:i:s');
      $result = D('course')->save($_POST);
      // 如果更新失败，直接返回...
      if( !$result ) return;
    }
    // 判断course_id和gather_id的有效性...
    if( !isset($_POST['gather_id']) || !isset($_POST['course_id']) || $_POST['course_id'] <= 0 )
      return;
    // 需要对日期格式进行转换...
    $_POST['start_time'] = strtotime($_POST['start_time']);
    $_POST['end_time'] = strtotime($_POST['end_time']);
    // 再将课表记录转发给对应的采集端...
    $dbSys = D('system')->field('transmit_addr,transmit_port')->find();
    $transmit = transmit_connect_server($dbSys['transmit_addr'], $dbSys['transmit_port']);
    // 转发课表记录命令 => 修改 或 添加...
    if( $transmit ) {
      // 获取转发节点的MAC地址...
      $map['gather_id'] = $_POST['gather_id'];
      $dbGather = D('gather')->where($map)->field('mac_addr')->find();
      // 组合课表数据成JSON...
      $_POST['mac_addr'] = $dbGather['mac_addr'];
      $saveJson = json_encode($_POST);
      // 发送转发命令...
      $json_data = transmit_command(kClientPHP, $nCmdType, $transmit, $saveJson);
      // 关闭中转服务器链接...
      transmit_disconnect_server($transmit);
      // 反馈转发结果...
      echo $json_data;
    }
  }
  //
  // 删除课程表数据...
  public function delCourse()
  {
    // 先直接删除数据库中记录...
    D('course')->where('course_id='.$_POST['course_id'])->delete();
    // 判断有没有采集端编号...
    if( !isset($_POST['gather_id']) || !isset($_POST['camera_id']) )
      return;
    // 再通知采集端...
    $dbSys = D('system')->field('transmit_addr,transmit_port')->find();
    $transmit = transmit_connect_server($dbSys['transmit_addr'], $dbSys['transmit_port']);
    if( $transmit ) {
      // 获取转发节点的MAC地址...
      $map['gather_id'] = $_POST['gather_id'];
      $dbGather = D('gather')->where($map)->field('mac_addr')->find();
      // 组合课表数据成JSON...
      $_POST['mac_addr'] = $dbGather['mac_addr'];
      $saveJson = json_encode($_POST);
      // 发送转发命令...
      $json_data = transmit_command(kClientPHP, kCmd_PHP_Set_Course_Del, $transmit, $saveJson);
      // 关闭中转服务器链接...
      transmit_disconnect_server($transmit);
      // 反馈转发结果...
      echo $json_data;
    }
  }
  //
  // 具体判断是否重叠的函数...
  private function IsTimeOverLaped(&$outItem)
  {
    // $_GET => /Admin/checkOverlap/camera_id/4/course_id/1/start_time/2017-04-07 23:0:30/end_time/2017-04-07 23:30:30/elapse_sec/1200/repeat_id/1
    // 解析传递过来的数据...
    $arrData = $_POST;
    $nCameraID = $arrData['camera_id'];
    $nCourseID = $arrData['course_id'];
    $tEnd = strtotime($arrData['end_time']);
    $tStart = strtotime($arrData['start_time']);
    $tDuration = intval($arrData['elapse_sec']);
    $nRepMode = intval($arrData['repeat_id']);
    // 查找当前摄像头(班级)下面所有的课程表记录...
    $map['camera_id'] = $nCameraID;
    $arrCourse = D('course')->where($map)->select();
    foreach ($arrCourse as &$dbItem) {
      $nItemID = intval($dbItem['course_id']);
      if( $nCourseID == $nItemID )
        continue;
      $outItem = $dbItem;
      $tItemStart = strtotime($dbItem['start_time']);
      $tItemEnd = strtotime($dbItem['end_time']);
      $tItemDur = intval($dbItem['elapse_sec']);
      $nItemRep = intval($dbItem['repeat_id']);
      if( $tStart < $tItemStart && $tEnd > $tItemStart )  // 区间 1
        return TRUE;
      if( $tStart < $tItemEnd && $tEnd > $tItemEnd )			// 区间 2
        return TRUE;
      if( $tStart >= $tItemStart && $tEnd <= $tItemEnd )	// 区间 3
        return TRUE;
      // 处理每天重复的情况...
      if(($nItemRep == kDayRepeat) || ($nRepMode == kDayRepeat)) {
        // 计算去掉日期的时间(合并到同一天)...
        $tMapStart = 60 * (60*date('H', $tItemStart)+date('i', $tItemStart))+date('s', $tItemStart);
        $tMapEnd	= $tMapStart + $tItemDur;
        // 计算去掉日期的时间(合并到同一天)...
        $tInStart = 60 * (60*date('H', $tStart)+date('i', $tStart))+date('s', $tStart);
        $tInEnd = $tInStart + $tDuration;
        if( $tInStart < $tMapStart && $tInEnd > $tMapStart )  // 区间 1
          return TRUE;
        if( $tInStart < $tMapEnd && $tInEnd > $tMapEnd )      // 区间 2
          return TRUE;
        if( $tInStart >= $tMapStart && $tInEnd <= $tMapEnd )  // 区间 3
          return TRUE;
      }
      // 处理每周重复的情况...
      if(($nItemRep == kWeekRepeat) || ($nRepMode == kWeekRepeat)) {
        // 合并到同一周...
        $tMapStart = 60 * (60*(24*date('w', $tItemStart)+date('H', $tItemStart))+date('i', $tItemStart))+date('s', $tItemStart);
        $tMapEnd = $tMapStart + $tItemDur;
        // 合并到同一周...
        $tInStart = 60 * (60*(24*date('w', $tStart)+date('H', $tStart))+date('i', $tStart))+date('s', $tStart);
        $tInEnd = $tInStart + $tDuration;
        if( $tInStart < $tMapStart && $tInEnd > $tMapStart )  // 区间 1
          return TRUE;
        if( $tInStart < $tMapEnd && $tInEnd > $tMapEnd )      // 区间 2
          return TRUE;
        if( $tInStart >= $tMapStart && $tInEnd <= $tMapEnd )  // 区间 3
          return TRUE;
        }
    }
    return FALSE;
  }
  //
  // 判断课表时间是否发生重叠...
  public function checkOverlap()
  {
    // 准备返回的对象...
    $arrErr['err_code'] = false;
    $arrErr['err_msg'] = "OK";
    // 验证是否发生了时间重叠...
    $theOverItem = array();
    $theWeek = array('周日','周一','周二','周三','周四','周五','周六');
    if( $this->IsTimeOverLaped($theOverItem) ) {
      $strRepeat = "无";
      switch($theOverItem['repeat_id']) {
        case 0:  $strRepeat = "无"; break;
        case 1:  $strRepeat = "每天"; break;
        case 2:  $strRepeat = "每周 " . $theWeek[date('w', $theOverItem['start_time'])];  break;
        default: $strRepeat = "无"; break;
      }
      $arrErr['err_code'] = true;
      $arrErr['err_msg'] = sprintf("当前时间发生重叠，重叠区间如下：<br>%s ~ %s<br>重复方式：%s", $theOverItem['start_time'], $theOverItem['end_time'], $strRepeat);
    }
    // 直接返回运行结果...
    echo json_encode($arrErr);    
  }
}
?>