<?php

function wysos_vo_util_make_config($suite_id){
	$sip_user = db_query("SELECT value FROM wra_suite_resources WHERE suite_id =:suite_id AND resource_type = 'sip_user'", array(':suite_id'=>$suite_id))->fetchField();
	
	$sip_password = db_query("SELECT value FROM wra_suite_resources WHERE suite_id = :suite_id AND resource_type = 'sip_password'", array(':suite_id'=>$suite_id))->fetchField();
	
	$mac = db_query("SELECT value FROM wra_suite_resources WHERE suite_id = :suite_id And resource_type = 'mac'", array(':suite_id'=>$suite_id))->fetchField();
	
	$row = db_query("SELECT * FROM wra_suites WHERE suite_id = :suite_id", array(':suite_id' => $suite_id))->fetchObject();
	
	$company = $row->company_name;
	$vm_extension = $row->suite_no;
	
	drupal_set_message('Programming '.$mac);
	// Create Grandstream Config text files
	
	$config_dir = '/home/wyregnet/gs_config/';
	
	$config_tpl= 'GXP1165-config_template.txt';
	$tftp_dir = '/tftpboot/';
	$cmd_encode = '/home/wyregnet/GS_CFG_GEN/bin/encode.sh';
	$cfgOutput = file_get_contents($config_dir.$config_tpl);

	$cfgOutput .= "\r\n";
	$cfgOutput .= "P3=".$company."\r\n";
	$cfgOutput .= "P270=".$company."\r\n";
	$cfgOutput .= "P33=".$vm_extension."\r\n";
	$cfgOutput .= "P35=".$sip_user."\r\n";
	$cfgOutput .= "P36=".$sip_user."\r\n";
	$cfgOutput .= "P34=".$sip_password."\r\n";
	$cfgTxtFName = sprintf("%s.txt", $mac);
	$cfgEncFName = sprintf("cfg%s", $mac);
//drupal_set_message($cfgOutput);	
	file_put_contents($config_dir.$cfgTxtFName, $config_dir.$cfgOutput);
	$shell_command = sprintf("%s %s %s %s", $cmd_encode, $mac, $config_dir.$cfgTxtFName, $config_dir.$cfgEncFName);
//drupal_set_message($shell_command);	
	shell_exec($shell_command);
	
	// Upload to wyisdn by ssh2
	$c_wyisdn = ssh2_connect('10.0.0.21', 22);
	ssh2_auth_password($c_wyisdn, 'root', 'k1mk1m');
	ssh2_scp_send($c_wyisdn, $config_dir.$cfgEncFName, $tftp_dir.$cfgEncFName);
	
	// Upload to sip.wyphone.net by ssh2
	$c_wyisdn = ssh2_connect('10.0.0.9', 22);
	ssh2_auth_password($c_wyisdn, 'root', 'QHbVx2EH');
	ssh2_scp_send($c_wyisdn, $config_dir.$cfgEncFName, $tftp_dir.$cfgEncFName);
	
	return(TRUE);
};