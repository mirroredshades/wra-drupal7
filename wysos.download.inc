<?php

function wysos_download_page($did)
{
	global $user;
	
	$download_url = 'wysos/download/';
	$download_path = $_SERVER['DOCUMENT_ROOT'].'/wra_dl/';
	
	// Check if user permitted
	if(db_query('SELECT COUNT(*) FROM wra_download 
	LEFT JOIN wra_download_uid ON wra_download.did=wra_download_uid.did
	WHERE wra_download.did=:did AND (wra_download.created_uid=:uid OR wra_download_uid.uid=:uid)', array(':did'=>$did, ':uid'=>$user->uid))->fetchField() == 0)
	{
		// Permissions Fail
		drupal_set_message('Permissions Fail');
		return;
	}
	// Do queries
	$row_dl = db_query('SELECT title, created FROM wra_download WHERE did=:did', array(':did'=>$did))->fetchObject();
	//$row_dl = db_fetch_object($r_dl);
	drupal_set_title($row_dl->title);
	
	$r_files = db_query("SELECT fid, file_title, file_name FROM wra_download_files WHERE did=:did ORDER BY priority ASC", array(':did'=>$did));
	foreach($r_files as $row_files)
	{
		$output = l($row_files->file_title,$download_url.$did.'/'.$row_files->fid).'<br/>';
	}
	
	return $output;
}
function wysos_download_add($title, $files)
{
	global $user;
	$did = db_insert('wra_download')
			->fields(array(
				'title'=> $title,
				'created_uid'=> $user->uid,
				'created'=>REQUEST_TIME,	
			))->execute();
	$count = 1;
	foreach($files as $key => $value)
	{
		$r_file = db_insert('wra_download_files')
			->fields(array(
				'did' => $did,
				'priority' => $count,
				'file_title' => $key,
				'file_name' => $value,
			))->execute();
		$count++;
	}
	return $did;
}
function wysos_download_file($fid) {
	if (! is_numeric($fid)) {
		return drupal_access_denied();
	} 
	$download_url = 'wysos/download/';
	$download_path = $_SERVER['DOCUMENT_ROOT'].'/wra_dl/';
	
	// Get file_name
	$file_name = db_query("SELECT file_name FROM wra_download_files WHERE fid=:fid", array(':fid'=>$fid))->fetchField();
	
	// Get Path from filing ID
	$full_path = $download_path.$file_name;
	$ext = strtolower(pathinfo($full_path,  PATHINFO_EXTENSION));
	if (!(is_dir($full_path)) && ($ext == 'pdf' || $ext == 'doc' || $ext == 'odt') ) {
		// Force Download
		header("Content-disposition: attachment; filename=\"$file_name\"");
		header("Content-type: application/octet-stream");
		// Send Headers
		header("Pragma: public");
	    header("Expires: 0");
    	header("Cache-Control: must-revalidate, post-check=0, pre-check=0");
    	header("Cache-Control: public");
    	header("Content-Description: File Transfer");
		// Get MIME Type
		$mime_type = file_get_mimetype($full_path);
		header('Content-Type: ' . $mime_type);
 		readfile($full_path);

	} else {
		return drupal_access_denied();
	}
}
function wysos_download_add_file($title, $file)
{
	global $user;
	
	//$r_dl = db_query("INSERT wra_download (title,created_uid,created) VALUES ('%s',%d,UNIX_TIMESTAMP())", $title, $user->uid);
	//$did = db_last_insert_id('wra_download','did');
	
	$did = db_insert('wra_download')
			->fields(array(
				'title'=> $title,
				'created_uid'=> $user->uid,
				'created'=>REQUEST_TIME,	
			))->execute();
	$count = 1;
	
	//$r_file = db_query("INSERT wra_download_files (did,priority,file_title,file_name) VALUES (%d,%d,'%s','%s')", $did, 1,$title,$file);
	$fid = db_insert('wra_download_files')
			->fields(array(
				'did'=> $did,
				'priority'=> 1,
				'file_title'=>$title,	
				'file_name'=>$file,	
			))->execute();
	
	//$fid = db_last_insert_id('wra_download','did');
	
	return $fid;
}
