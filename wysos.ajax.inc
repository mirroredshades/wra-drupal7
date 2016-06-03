<?php 
function wysos_zone_list(){
	$cid = $_REQUEST['cid'];
	$zones = db_query("SELECT zone_id, zone_name FROM uc_zones WHERE zone_country_id = :cid",array(':cid'=>$cid))->fetchAll();
	//$codeval = db_select('uc_zones' , 'pc');
	//$codeval->fields('pc');
	//$codeval->condition('zone_country_id','active','=');
	//$r1 = $codeval->execute()->fetchAll();
	$zone_list = array();
	foreach($zones as $zone){
		$zone_list[] = array(
			'id' => $zone->zone_id,
			'name' => $zone->zone_name,
		);
	}
	if($zone_list){
	echo json_encode($zone_list);
	}
	else{
	echo 'invalid';
	}
}