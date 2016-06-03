<?php
function form_wysos_configure(){
	$form['cid'] = array(
		'#title' => t('CID'),
		'#type' => 'textfield',
		'#length' => 14,
		'#default_value' => '2003-000457827',
	);
	$form['cid_btn'] = array(
		'#type' => 'submit',
		'#id' => 'cid_btn',
		'#value' => t('Get Company By CID'),
	);
	$form['efnum'] = array(
		'#type' => 'textfield',
		'#title' => t('eFNum'),
		'#length' => 64,
		'#default_value' => '007220187052012030044229071022093004196002173131',	
	);
	$form['efnum_btn'] = array(
		'#type' => 'submit',
		'#id' => 'efnum_btn',
		'#value' => t('Get RA By eFNum'),
	);
	$form['dl_sos'] = array(
		'#type' => 'submit',
		'#id' => 'dl_sos_btn',
		'#value' =>t('Download SoS File'),	
	);
	$form['ul_dir'] = array(
		'#type' => 'textfield',
		'#title' => t('Upload Directory'),
	);
	$form['ul_dir_btn'] = array(
		'#type' => 'submit',
		'#id' => 'ul_dir_btn',
		'#value' => t('Update upload directory'),
	);
	return $form;
}
function form_wysos_configure_validate(){

}
function form_wysos_configure_submit($form, &$form_state){
	drupal_set_message('clicked_button: '.$form_state['clicked_button']['#id']);
	include_once("wysos.scrape-util.inc");
	switch($form_state['clicked_button']['#id']){
		case 'cid_btn' :
			$efnum = wysos_get_efnum_from_cid($form_state['values']['cid']);
			drupal_set_message('eFnum='.$efnum);
			break;
		case 'efnum_btn' :
			$ra_id = wysos_get_ra_from_efnum($form_state['values']['efnum']);
			drupal_set_message('ra_id='.$ra_id);
			break;
		case 'dl_sos_btn' :
			wysos_download_sos_file();
			break;
	}
}
/*
 * Export users for stamps.com
 */
function wysos_export_users(){
	$basefile = time().mt_rand(0,mt_getrandmax()).'csv';
	echo $basefile;
	






}









