<?php
function wysos_resolution_form($form,&$form_state){
	$form['user_name'] = array(
		'#title' => t('Username'),
		'#type' => 'textfield',
		'#size' => '30',
		'#maxlength' => '60',
		'#autocomplete_path' =>'user/autocomplete',
		'#description' => t('Please enter username.'),
	);
	$form['filing_name'] = array(
		'#title' => t('Filing Name'),
		'#type' => 'textfield',
		'#size' => '50',
		'#maxlength' => '80',
		'#default_value' => isset($form_state['values']['filing_name'])?$form_state['values']['filing_name']:'',
		'#autocomplete_path' => 'wysos/entity/autocomplete',
		'#required' => TRUE,
		'#description' => t('Please enter company name'),
	);
	
	/*if(!empty($form_state['values']['filing_name'])){
		$filing_type = db_select('WySosFILING','w')
				->fields('w',array('FILING_TYPE'))
				->condition('w.FILING_NAME',$form_state['values']['filing_name'],'=')
				->execute()->fetchField();
	}*/
	if(!empty($form_state['values']['filing_name'])) {
		$filing_type = db_query("SELECT FILING_TYPE FROM WySosFILING WHERE FILING_NAME=:FILING_NAME", array(':FILING_NAME'=>$form_state['values']['filing_name']))->fetchField();
		if (empty($filing_type)) {
			if (!empty($form_state['values']['filing_type'])) {
				$filing_type = $form_state['values']['filing_type'];
			}
		}
	}
	$form['filing_type'] = array(
		'#title' => t('Filing Type'),
		'#type' => 'select',
		'#required' => TRUE,
		'#options' => array(''=>'','CORP'=>'Corporation', 'LP'=>'LP', 'LLC'=>'LLC', 'NPCORP'=>'Non Profit Corp'),
		'#default_value' => isset($filing_type)?$filing_type:'',
	);
	$form['resolution_form'] = array(
		'#title' => t('Resolution'),
		'#type' => 'select',
		'#required' => TRUE,
		'#options' => array(''=>'','AppointDirector'=>'Appoint Initial Director','ResignIncorp'=>'Resignation of Incorporator','ResignNominee'=>'Resignation of Nominee (Dated)','ResignNomineeUndated'=>'Resignation of Nominee (Undated)','BankRes'=>'Banking Resolution','Welcome'=>'Welcome Letter','OwnershipStatement'=>'Ownership Statement', '29-ResignIncorp' => '29Dollar Resignation'),
		'#default_value' => isset($form_state['values']['resolution_form'])?$form_state['values']['resolution_form']:'',
	);
	$form['director1'] = array(
		'#title' => t('Director/Manager #1'),
		'#type' => 'textfield',
		'#size' => 40,
		'#maxlength' => '80',
		'#default_value' => isset($form_state['values']['director1'])?$form_state['values']['director1']:'',
	);
	$form['director1_address'] = array(
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '80',
        '#default_value' => isset($form_state['values']['director1_address']) ? $form_state['values']['director1_address'] :'',
	);
	$form['director2'] = array(
		'#title' => t('Director/Manager #2'),
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '80',
        '#default_value' => isset($form_state['values']['director2']) ? $form_state['values']['director2'] :'',
	);
	$form['director2_address'] = array(
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '80',
        '#default_value' => isset($form_state['values']['director2_address']) ? $form_state['values']['director2_address'] :'',
	);
	$form['director3'] = array(
		'#title' => t('Director/Manager #3'),
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '80',
        '#default_value' => isset($form_state['values']['director3']) ? $form_state['values']['director3'] :'',
	);
	$form['director3_address'] = array(
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '80',
        '#default_value' => isset($form_state['values']['director3_address']) ? $form_state['values']['director3_address'] :'',
	);
	$form['director4'] = array(
		'#title' => t('Director/Manager #4'),
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '80',
        '#default_value' => isset($form_state['values']['director4']) ? $form_state['values']['director4'] :'',
	);
	$form['director4_address'] = array(
        '#type' => 'textfield',
        '#size' => '40',
		'#maxlength' => '80',
        '#default_value' => isset($form_state['values']['director4_address']) ? $form_state['values']['director4_address'] :'',
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Update'
		);
	return $form;
}

function wysos_resolution_form_submit($form, &$form_state)
{
	// Set values based on order type
	include_once('wysos.util.inc');

	if($form_state['values']['resolution_form'] == 'Welcome')
		$doc_name = 'Welcome.odt';
	else
		$doc_name = $form_state['values']['filing_type']. '-'. $form_state['values']['resolution_form'].'.odt';

	//$account = user_load(array('name' => $form_state['values']['user_name']));
	$account = user_load_by_name($form_state['values']['user_name']);
	if($account) {
		$result = db_query('SELECT f.name, f.type, v.value FROM {profile_field} f INNER JOIN {profile_value} v ON f.fid = v.fid WHERE uid = :uid', array(':uid'=>$account->uid))->fetchAll();
		foreach ($result as $field) {
			if (empty($GLOBALS[$field->name])) {
				$GLOBALS[$field->name] = _profile_field_serialize($field->type) ? unserialize($field->value) : $field->value;
			}
		}
	}
	$GLOBALS['company_name'] = $form_state['values']['filing_name'];
	$GLOBALS['username'] = $account->name;
	$GLOBALS['password'] = $account->name.'11';
	$data = array();
	$GLOBALS['director1'] = $form_state['values']['director1'];
	$data[] = array('name'=> $form_state['values']['director1'], 'address'=> $form_state['values']['director1_address']);
	if(!empty($form_state['values']['director2'])) {
		$GLOBALS['director2'] = $form_state['values']['director2'];
		$data[] = array('name'=> $form_state['values']['director2'], 'address'=> $form_state['values']['director2_address']);
	}
	if(!empty($form_state['values']['director3'])) {
		$GLOBALS['director3'] = $form_state['values']['director3'];
		$data[] = array('name'=> $form_state['values']['director3'], 'address'=> $form_state['values']['director3_address']);
	}
	if(!empty($form_state['values']['director4'])) {
		$GLOBALS['director4'] = $form_state['values']['director4'];
		$data[] = array('name'=> $form_state['values']['director4'], 'address'=> $form_state['values']['director4_address']);
	}
//	drupal_set_message('data:' . print_r($data, TRUE));
	$filename = filesystem_base_path()."/sites/all/modules/wysos/test1.odt";
	$handle = fopen($filename, "r");
	$contents = fread($handle, filesize($filename));
	
	fclose($handle);
	$handle = tmpfile();
	fwrite($handle, $contents);

	// Open the OpenOffice Template
	include_once('tbs_class.php');
	include_once('tbs_plugin_opentbs.php');

	$TBS = new clsTinyButStrong;
	$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);

	$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
	$TBS->PlugIn(OPENTBS_SELECT_MAIN);	
	
	$TBS->MergeBlock('a', $data);
	//$TBS->Show(OPENTBS_DOWNLOAD, $form_state['values']['resolution_form'].'-'.$form_state['values']['filing_name'].'.odt');
	$TBS->Show(); 
	
}
function wysos_op_agreement_form($form, &$form_state){
	if(empty($form_state['storage']['step'])){
		$form_state['storage']['step'] = 1;
	}
	include_once('wysos.util.inc');
	
	switch($form_state['storage']['step']){
		case 1:
			$form['filing_name'] = array(
				'#title' => t('Filing Name'),
				'#type' => 'textfield',
				'#size' => '50',
				'#maxlength' => '80',
				'#autocomplete_path' => 'wysos/entity/autocomplete',
				'#default_value' => isset($form_state['storage']['values'][1]['filing_name'])?$form_state['storage']['values'][1]['filing_name']:'',
				'#required' => TRUE,
				'#description' => t('Please enter company name.'),
			);
			$form['num_members'] = array(
				'#title' => t('Number Members'),
				'#type' => 'textfield',
				'#required' => TRUE,
				'#size' => '3',
				'#maxlength' => '3',
				'#default_value' => isset($form_state['storage']['values'][1]['num_members'])? $form_state['storage']['values'][1]['num_managers']:1,
			);
			$form['num_managers'] = array (
				'#title' => t('Number Managers'),
				'#type' => 'textfield',
				'#required' => TRUE,
				'#size' => '3',
				'#maxlength' => '3',
				'#default_value' => isset($form_state['storage']['values'][1]['num_managers'])?$form_state['storage']['values'][1]['num_managers']:1,
			);
			$form['agreement_date'] = array(
				'#title' => t('Started Date'),
				'#type' => 'date',
				'#required' => TRUE,
		        '#default_value' => isset($form_state['storage']['values'][1]['started_dt'])?$form_state['storage']['values'][1]['started_dt']:'',
			);
			break;
		case 2:
			for($i= 1;$i<= $form_state['storage']['values'][1]['num_members'];$i++){
				$form['member_name'.$i] = array(
					'#title' => t('Member #'.$i),
					'#description' => t('Name'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '80',
					'#default_value' => isset($form_state['storage']['values'][2]['member_name'.$i])?$form_state['storage']['values'][2]['member_name'.$i]:'',
				);
				$form['member_address'.$i] = array(
					'#description' => t('Address'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '80',
					'#default_value' => isset($form_state['storage']['values'][2]['member_address'.$i])?$form_state['storage']['values'][2]['member_address'.$i]:'',
				);
				if($form_state['storage']['values'][1]['num_members'] > 1) {
					$form['member_percent'.$i] = array(
						'#description' => t('Ownership %'),
						'#type' => 'textfield',
						'#size' => '3',
						'#maxlength' => '3',
						'#default_value' => isset($form_state['storage']['values'][2]['member_percent'.$i])?$form_state['storage']['values'][2]['member_percent'.$i]:'',
					);
				}
			}
			break;
			case 3:
			for ($i=1; $i<=$form_state['storage']['values'][1]['num_managers']; $i++)
			{
				$form['manager_name'.$i] = array(
					'#title' => t('Manager #'.$i),
					'#description' => t('Name'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '80',
					'#default_value' => isset($form_state['storage']['values'][2]['manager_name'.$i])?
					$form_state['storage']['values'][2]['manager_name'.$i]:'',
				);
				$form['manager_address'.$i] = array(
					'#description' => t('Address'),
					'#type' => 'textfield',
					'#size' => '40',
					'#maxlength' => '80',
					'#default_value' => isset($form_state['storage']['values'][2]['manager_address'.$i])? $form_state['storage']['values'][2]['manager_address'.$i]:'',
				);
			}
			break;
	}
	if($form_state['storage']['step'] > 1) {
		$form['previous'] = array(
			'#type' => 'submit',
			'#value' => '<< Previous' 
			);
	}
	$form['next'] = array(
		'#type' => 'submit',
		'#value' => 'Next >>'
		);
	return $form;
}
function wysos_op_agreement_form_validate($form, &$form_state)
{
	switch($form_state['storage']['step']) {
		case 1:
			break;

		case 2:
			break;

		case 3:
			break;
	};
}

function wysos_op_agreement_form_submit($form, &$form_state)
{
	$form_state['storage']['values'][$form_state['storage']['step']] = $form_state['values'];
	$create_op_agreement = FALSE;
	switch($form_state['storage']['step']) {
		case 1:
				$form_state['storage']['step']++;
			break;

		case 2:
			if($form_state['clicked_button']['#id'] == 'edit-previous') {
				$form_state['storage']['step']--;
			} elseif($form_state['clicked_button']['#id'] == 'edit-next') {
				if($form_state['storage']['values'][1]['num_members'] == 1) 
					$create_op_agreement = TRUE;
				else
					$form_state['storage']['step']++;
			}
			break;

		case 3:
			if($form_state['clicked_button']['#id'] == 'edit-previous') {
				$form_state['storage']['step']--;
			} elseif($form_state['clicked_button']['#id'] == 'edit-next') {
				// Create the Operating Agreement
				$create_op_agreement = TRUE;
			}
			break;
	};


	if($create_op_agreement)
	{	
		// Open the OpenOffice Template
		include_once('tbs_class.php');
		include_once('tbs_plugin_opentbs.php');
	
		$TBS = new clsTinyButStrong;
		$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);

		$GLOBALS['company_name'] = $form_state['storage']['values'][1]['filing_name'];
		$GLOBALS['agreement_date'] = strftime('%B %e, %Y', mktime(0, 0, 0, $form_state['storage']['values'][1]['agreement_date']['month'],$form_state['storage']['values'][1]['agreement_date']['day'], $form_state['storage']['values'][1]['agreement_date']['year']));
		if($form_state['storage']['values'][1]['num_members'] == 1)
		{
			$doc_name = "Operating Agreement (Single Member).odt";
			$GLOBALS['member_name1'] = $form_state['storage']['values'][2]['member_name1'];
			$GLOBALS['member_address1'] = $form_state['storage']['values'][2]['member_address1'];

			drupal_set_message('values:' . print_r($form_state['storage']['values'], TRUE));
			
		} else {
			$doc_name = "Operating Agreement (Multi Member).odt";
			$data = array();
			$GLOBALS['director1'] = isset($form_state['values']['director1'])?$form_state['values']['director1']:'';
			$data[] = array('name'=> $GLOBALS['director1'], 'address'=> isset($form_state['values']['director1_address'])?$form_state['values']['director1_address']:'');
			if(!empty($form_state['values']['director2'])) {
				$GLOBALS['director2'] = $form_state['values']['director2'];
				$data[] = array('name'=> $form_state['values']['director2'], 'address'=> $form_state['values']['director2_address']);
			}
			if(!empty($form_state['values']['director3'])) {
				$GLOBALS['director3'] = $form_state['values']['director3'];
				$data[] = array('name'=> $form_state['values']['director3'], 'address'=> $form_state['values']['director3_address']);
			}
			if(!empty($form_state['values']['director4'])) {
				$GLOBALS['director4'] = $form_state['values']['director4'];
				$data[] = array('name'=> $form_state['values']['director4'], 'address'=> $form_state['values']['director4_address']);
			}
//			$TBS->MergeBlock('a', $data);
		}
	
	
		$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
		$TBS->Show(TBS_OUTPUT, 'Operating Agreement-'.$form_state['storage']['values'][1]['company_name'].'.odt'); 
	}
	
	$form_state['rebuild'] = TRUE;

}

function filesystem_base_path()
{
    if(!isset($GLOBALS['filesystem_base_path']))
    {
      $search = "includes".DIRECTORY_SEPARATOR."bootstrap.inc"; 
	  // Walk directories up until we find the $search-path (wich is relative to root)
      for($path=dirname(__FILE__); !file_exists($path.DIRECTORY_SEPARATOR.$search); $path.= DIRECTORY_SEPARATOR."..")
      {
        // no need to do anything
      }
      // store the path if it one was found
      $GLOBALS['filesystem_base_path'] = realpath($path);
    }
    return $GLOBALS['filesystem_base_path'];
}
