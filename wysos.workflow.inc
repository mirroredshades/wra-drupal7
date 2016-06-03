<?php

function wysos_workflow_list_form($form, &$form_state, $category){
	$options = array();
	
	$options['/wysos/workflow/list/all'] = 'All';
	
	$form['URL'] = array(
		'#type' => 'select',
		'#default_value' => '/wysos/workflow/list/'.arg(3),
		'#options' => $options,
		'#attributes' => array('onchange' => 'window.location.href=this.form.URL.options[this.form.URL.selectedIndex].value'),
	);
	$form['list_wrapper'] = array(
		'#type' => 'item',
		'#prefix' => '<div id="list-wrapper">',
		'#markup' => wysos_get_workflow_list2($category),
		'#suffix' => '</div>',
	);
	return $form;
}

function wysos_get_workflow_list2($category){
	//Table Header
	
	$headers = array(
		array(
			'data' => t('Order'),
			'field' => 'order_id',
			'sort' => 'desc'),
		array(
			'data' => t('Submitted'),
			'field' => 'created'),
		array(
			'data' => t('Status'),
			'field' => 'order_status'),
		array(
			'data' => t('Total'),
			'field' => 'order_total'),
		array(
			'data' => t('Customer'),
			'field' => 'customer'),
		array(
			'data' => t('Company'),
			'field' => 'company_name'),
			'Filing'
	);
	//Generate table
	if($category == 'all'){
		$query = "SELECT order_id, uc_orders.uid, order_status, order_total, LEFT(CONCAT(billing_first_name, CONCAT(' ',billing_last_name)),16) AS customer, LEFT(billing_company,16) AS company_name, uc_orders.created FROM uc_orders 
		INNER JOIN uc_order_statuses ON uc_orders.order_status = uc_order_statuses.order_status_id WHERE uc_orders.created > UNIX_TIMESTAMP()- 2629743 AND uc_order_statuses.weight >= 5 ORDER BY order_id DESC";
	}
	else {
		$query = "SELECT order_id,uid, order_status, order_total, LEFT(CONCAT(billing_first_name, CONCAT(' ',billing_last_name)),16) AS customer, LEFT(billing_company,16) AS company_name, uc_orders.created FROM uc_orders WHERE uc_orders.order_status = $category AND uc_orders.created > UNIX_TIMESTAMP() - 15518972 ORDER BY order_id DESC";		
	}
	$result = db_query($query)->fetchAll();
	
	$options = $rows = array();
	foreach ($result as $dbrow){
		$rows[] = array(l($dbrow->order_id, 'admin/store/orders/'.$dbrow->order_id),
				date('m-d-Y',$dbrow->created),substr($dbrow->order_status, 0, 10),
				'$'.number_format($dbrow->order_total,2),
				l($dbrow->customer,'user/'.$dbrow->uid),
				$dbrow->company_name, wysos_workflow_opts2($dbrow));
	}
	
	$output = theme('table', array('header'=> $headers, 'rows'=>$rows));
	return $output; 
}

function wysos_workflow_opts2($row){
	$output = array();
	if($row->order_status == 'payment_received' || $row->order_status == 'filing_printed'){
		$count = 1;
		$result = db_query("SELECT order_product_id, model, data FROM uc_order_products WHERE order_id=$row->order_id AND model IN ('WYP1R', 'WYLLC1R', 'WYP', 'CLOSE', 'WYCLOSE', 'CLLC')");
		$form[] = array(
			'#value' => '<table>',
			'#weight' => $count++,
		);
		foreach ($result as $product){
			$data = unserialize($product->data);
			$form[] = array(
				'#type' => 'item',
				'#markup' => isset($data['attributes']['Name of the company'][0])?substr($data['attributes']['Name of the company'][0], 0, 16):'',
				'#prefix' => '<tr><td>',
				'#suffix' => '</td>',
				'#weight' => $count++,			
			);
			$options = array();
			$options[''] = t('Select Action');
			switch($product->model){
				case 'WYP1R' :
				case 'WYP' :
					if($row->order_status == 'payment_received'){
						if($data['attributes']['Nominee'][0] == '(none)' || empty($data['attributes']['Nominee'][0])){
							$options['/wysos/workflow/articles/1/'.$product->order_product_id] = 'Articles';
							$options['/wysos/workflow/articles/2/'.$product->order_product_id] = 'Articles(Simple)';		
						}else{
							$options['/wysos/workflow/articles/3/'.$product->order_product_id] = 'Articles';
							$options['/wysos/workflow/articles/2/'.$product->order_product_id] = 'Articles(Simple)';
						}
					}else if($row->order_status == 'filing_printed'){
						$options['/wysos/workflow/resolutions'.$product->order_product_id] = 'Resolutions';					
					}else if($row->order_status == 'submit_to_state'){
						$options['/wysos/workflow/articles_approved/'.$product->order_product_id] = 'Filing Approved';
						$options['/wysos/workflow/articles_rejected'.$product->order_product_id] = 'Filing Rejected';
					}else if($row->order_status == 'wait_mailing'){
						$options['/wysos/workflow/mail_out/'.$product->order_product_id] = 'Mail Out';
					}
					break;
					
					case 'WYLLC1R' :
					case 'WyLLC' :
						if($row->order_status == 'payment_received'){
							if($data['attributes']['Nominee'][0] == '(none)' || empty($data['attributes']['Nominee'][0])){
								$options['/wysos/workflow/articles/4/'.$product->order_product_id] = 'Articles';
								$options['/wysos/workflow/articles/5/'.$product->order_product_id] = 'Articles (Simple)';
							}else{
								$options['/wysos/workflow/articles/6/'.$product->order_product_id] = 'Articles';
								$options['/wysos/workflow/articles/5/'.$product->order_product_id] = 'Articles (Simple)';
							}
						}else if($row->order_status == 'filing_printed'){
							$options['/wysos/workflow/resolutions/'.$product->order_product_id] = 'Resolutions';
						}
						break;
					case 'LP' :
						if($row->order_status == 'payment_received'){
							if($data['attributes']['Nominee'][0] == '(none)' || empty($data['attributes']['Nominee'][0])){
								$options['/wysos/workflow/articles/7/'.$product->order_product_id] = 'Articles';
							}else{
								$options['/wysos/workflow/articles/8/'.$product->order_product_id] = 'Articles';
							}
						}else if($row->order_status == 'filing_printed'){
							$options['/wysos/workflow/resolutions'.$product->order_product_id] = 'Resolutions';
						}
						break;
					
					case 'CLOSE' :
					case 'WYCLOSE' :
						if($row->order_status == 'payment_received'){
							if($data['attributes']['Nominee'][0] == '(none)' || empty($data['attributes']['Nominee'][0])){
								$options['/wysos/workflow/articles/9/'.$product->order_product_id] = 'Articles';
								$options['/wysos/workflow/articles/10/'.$product->order_product_id] = 'Articles (Simple)';
							}else{
								$options['/wysos/workflow/articles/11/'.$product->order_product_id] = 'Articles';
								$options['/wysos/workflow/articles/10/'.$product->order_product_id] = 'Articles (Simple)';
							}
						}else if($row->order_status == 'filing_printed'){
							$options['/wysos/workflow/resolutions/'.$product->order_product_id] = 'Resolutions';
						}
						break;
					case 'CLLC' :
						if($row->order_status == 'payment_received'){
							if($data['attributes']['Nominee'][0] == '(none)' || empty($data['attributes']['Nominee'][0])){
								$options['/wysos/workflow/articles/12/'.$product->order_product_id] = 'Articles';
								$options['/wysos/workflow/articles/13/'.$product->order_product_id] = 'Articles (Simple)';
							}else{
								$options['/wysos/workflow/articles/14/'.$product->order_product_id] = 'Articles';
								$options['/wysos/workflow/articles/13/'.$product->order_product_id] = 'Articles (Simple)';
							}
						}else if($row->order_status == 'filing_printed'){
							$options['/wysos/workflow/resolutions'.$product->order_product_id] = 'Resolutions';
						}
						break;
			}	
		
			$form[] = array(
				'#type' => 'select',
				'#default_value' => '',
				'#name' => 'select'.$product->order_product_id,
				'#prefix' => '<td>',
				'#suffix' => '</td>',
				'#options' => $options,
				'#weight' => $count++,
			);
			$form[] = array(
				'#name' => 'btn'.$product->order_product_id,
				'#value' => 'Go',
				'#type' => 'button',
				'#prefix' => '<td>',
				'#suffix' => '</td>',
				'#weight' => $count++,
				'#attributes' => array('onclick' => 'window.location,href = this.form.select'. $product->order_product_id.'.options[this.form.select'.$product->order_product_id.'.selectedIndex].value; return false;'),
			);
		}
		$form[] = array(
			'#value' => '</table>',
			'#weight' => $count++,
		);
		$output = drupal_render($form);
	}
	return $output;
}

function wysos_workflow_articles2($articleid, $orderproductid){
	
	global $user;
	// Article Associative Array
	$articles[1] = 'Articles-CORP-Officers.odt';
	$articles[2] = 'Articles-CORP-Simple.odt';
	$articles[3] = 'Articles-CORP-Nominee.odt';
	$articles[4] = 'Articles-LLC-Managers.odt';
	$articles[5] = 'Articles-LLC-Simple.odt';
	$articles[6] = 'Articles-LLC-Nominee.odt';
	$articles[7] = 'Articles-LP-Partners.odt';
	$articles[8] = 'Articles-LP-Nominee.odt';
	$articles[9] =  'Articles-CLOSE-Officers.odt';
	$articles[10] = 'Articles-CLOSE-Simple.odt';
	$articles[11] = 'Articles-CLOSE-Nominee.odt';
	$articles[12] = 'Articles-CLLC-Managers.odt';
	$articles[13] = 'Articles-CLLC-Simple.odt';
	$articles[14] = 'Articles-CLLC-Nominee.odt';
	$doc_name = $articles[$articleid]; 
	
	// Get Order Product
	$product = db_select('uc_order_products','u')
		->fields('u')
		->condition('u.order_product_id',$orderproductid, '=')
		->execute()->fetchObject();
	if(!$product){
		return drupal_access_denied();
	}
	$data = unserialize($product->data);
	
	switch($product->model){
		case 'WYP1R' :
		case 'WYP' :
			//Profit Corporation
			$company_name = $data['attributes']['Name of the company'][0];
			$GLOBALS['company_name'] = $data['attributes']['Name of the company'][0];
			if(is_array($data['attributes']['Authorized Shares'])){
				$GLOBALS['num_shares'] = $data['attributes']['Authorized Shares'][0];
			}else{
				$GLOBALS['num_shares'] = $data['attributes']['Authorized Shares'];
			}
			if(is_array($data['attributes']['Share Class'])){
				$GLOBALS['share_class'] = $data['attributes']['Share Class'][0];
			}else{
				$GLOBALS['share_class'] = $data['attributes']['Share Class'];
			}
			if(is_array($data['attributes']['Par Value'])){
				$par_value = $data['attributes']['Par Value'][0];
			}else{
				$par_value = $data['attributes']['Par Value'];
			}
			if($par_value == 'No Par Value'){
				$GLOBALS['share_par'] = 'No';
			}else{
				$GLOBALS['share_par'] = $par_value;
			}
			$data2 = array();
			$data2[] = array('name'=>$data['attributes']['Director #1 Name'][0], $address=>$data['attributes']['Director #1 Address'][0]);
			if(!empty($data['attributes']['Director #2 Name'][0])){
				$data[] = array('name'=>$data['attributes']['Director #2 Name'][0], $address=>$data['attributes']['Director #2 Address'][0]);
			}
			if(!empty($data['attributes']['Director #3 Name'][0])){
				$data[] = array('name'=>$data['attributes']['Director #3 Name'][0], $address=>$data['attributes']['Director #3 Address'][0]);
			}
			if(!empty($data['attributes']['Director #4 Name'][0])){
				$data[] = array('name'=>$data['attributes']['Director #4 Name'][0], $address=>$data['attributes']['Director #4 Address'][0]);
			}
			$result = db_update('uc_orders')
					->fields(array('order_status'=>'filing_printed'))
					->condition('order_id',$orderid,'=')
					->execute();
			
			uc_order_comment_save($product->order_id, $user->uid, 'Printed Articles of Incorporation for '.$company_name,'order');
			$result = db_update('uc_orders')
					->fields(array('order_status'=>'filing_printed'))
					->condition('order_id',$product->order_id)
					->execute();
			include_once('tbs_class.php');
			include_once('tbs_plugin_opentbs.php');
			
			$TBS = new clsTinyButStrong;
			$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);

			$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
			$TBS->MergeBlock('a', $data2);
			$TBS->Show(OPENTBS_DOWNLOAD, 'Articles-'.$company_name.'.odt');			
			break;		
			
			case 'CLOSE' :
			case 'WYCLOSE' :
				//Close Corporation
				$company_name = $data['attributes']['Name of the company'][0];
				$GLOBALS['company_name'] = $data['attributes']['Name of the company'][0];
				if(is_array($data['attributes']['Authorized Shares'])){
					$GLOBALS['num_shares'] = $data['attributes']['Authorized Shares'][0];
				}else{
					$GLOBALS['num_shares'] = $data['attributes']['Authorized Shares']; 
				}
				if(is_array($data['attributes']['Share Class'])){
					$GLOBALS['share_class'] = $data['attributes']['Share Class'][0];
				}else{
					$GLOBALS['share_class'] = $data['attributes']['Share Class'];
				}
				if(is_array($data['attributes']['Par Value of Shares'])){
					$par_value = $data['attributes']['Par Value'][0];
				}else{
					$par_value = $data['attributes']['Par Value'];
				}
				if($par_value == 'No Par Value'){
					$GLOBALS['share_par'] = 'No';
				}else{
					$GLOBALS['share_par'] = $par_value;
				}
				$data2 = array();
				$data2[] = array('name' => $data['attributes']['Director #1 Name'][0], 'address' => $data['attributes']['Director #1 Address'][0]);
				if(!empty($data['attributes']['Director #2 Name'])){
					$data2[] = array('name' => $data['attributes']['Director #2 Name'][0], 'address' => $data['attributes']['Director #2 Address'][0]);
				}
				if(!empty($data['attributes']['Director #3 Name'])){
					$data2[] = array('name' => $data['attributes']['Director #3 Name'][0], 'address'=> $data['attributes']['Director #3 Address'][0]);
				}
				if(!empty($data['attributes']['Director #4 Name'])){
					$data2[] = array('name'=> $data['attributes']['Director #4 Name'][0], 'address'=> $data['attributes']['Director #4 Address'][0]);
				}
				
				$result = db_update('uc_orders')
						->fields(array('order_status'=>'filing_printed'))
						->condition('order_id', $orderid, '=')
						->execute();
				uc_order_comment_save($product->order_id, $user->uid, 'Printed Articles of Incorporation for'.$company_name,'order');
				
				$result = db_update('uc_orders')
						->fields(array('order_status'=>'filing_printed'))
						->condition('order_id', $product->order_id, '=')
						->execute();
						
				include_once('tbs_class.php');
				include_once('tbs_plugin_opentbs.php');
				
				$TBS = new clsTinyButStrong;
				$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
				
				$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
				$TBS->MergeBlock('a', $data2);
				$TBS->Show(OPENTBS_DOWNLOAD, 'Articles-'.$company_name.'.odt');
				break;
			case 'WYLLC' :
			case 'WYLLC1R' :
				// LLC
				$company_name = $data['attributes']['Name of the company'][0];
				if(stripos($company_name,' LLC')===false && stripos($company_name,' LC')===false && stripos($company_name,' Limited Liability Company')===false 
					&& stripos($company_name,' Ltd. Liability Company')===false && stripos($company_name,' L.L.C.')===false 
					&& stripos($company_name,' Limited Company')===false){
						$company_name = trim($company_name).' LLC';
					}
				$GLOBALS['company_name'] = $company_name;
				$data2 = array();
				$data2[] = array('name'=>$data['attributes']['Manager #1 Name'][0], 'address'=>$data['attributes']['Manager #1 Address'][0]);
				if(!empty($data['attributes']['Manager #2 Name'][0])) 
					$data2[] = array('name'=>$data['attributes']['Manager #2 Name'][0], 'address'=>$data['attributes']['Manager #2 Address'][0]);
				if(!empty($data['attributes']['Manager #3 Name'][0])) 
					$data2[] = array('name'=>$data['attributes']['Manager #3 Name'][0], 'address'=>$data['attributes']['Manager #3 Address'][0]);
				if(!empty($data['attributes']['Manager #4 Name'][0])) 
					$data2[] = array('name'=>$data['attributes']['Manager #4 Name'][0], 'address'=>$data['attributes']['Manager #4 Address'][0]);
				$result = db_update('uc_orders')
						->fields(array('order_status' => 'filing_printed'))
						->condition('order_id', $orderid, '=')
						->execute();
				uc_order_comment_save($product->order_id, $user->uid, 'Printed Articles of Organization for '.$company_name,'order');
				$result = db_update('uc_orders')
						->fields(array('order_status' => 'filing_printed'))
						->condition('order_id', $product->order_id, '=')
						->execute();
				include_once('tbs_class.php');
				include_once('tbs_plugin_opentbs.php');
				
				$TBS = new clsTinyButStrong;
				$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
				
				$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
				$TBS->MergeBlock('a',$data2);
				$TBS->Show(OPENTBS_DOWNLOAD, 'Articles-'.$company_name.'.odt');
				break;
			case 'CLLC' :
				// Close LLC
				$company_name = $data['attributes']['Name of the company'][0];
				if(stripos($company_name,' LLC')===false && stripos($company_name,' LC')===false && stripos($company_name,' Limited Liability Company')===false 
					&& stripos($company_name,' Ltd. Liability Company')===false && stripos($company_name,' L.L.C.')===false 
					&& stripos($company_name,' Limited Company')===false) {
					$company_name = trim($company_name).' LLC';
				}
				$GLOBALS['company_name'] = $company_name;
				$data = array();
				$data[] = array('name'=>$data['attributes']['Manager #1 Name'][0], 'address'=>$data['attributes']['Manager #1 Address'][0]);
				if(!empty($data['attributes']['Manager #2 Name'][0])) 
					$data[] = array('name'=>$data['attributes']['Manager #2 Name'][0], 'address'=>$value->data['attributes']['Manager #2 Address'][0]);
				if(!empty($data['attributes']['Manager #3 Name'][0])) 
					$data[] = array('name'=>$data['attributes']['Manager #3 Name'][0], 'address'=>$data['attributes']['Manager #3 Address'][0]);
				if(!empty($data['attributes']['Manager #4 Name'][0])) 
					$data[] = array('name'=>$data['attributes']['Manager #4 Name'][0], 'address'=>$data['attributes']['Manager #4 Address'][0]);
					
				$result = db_update('uc_orders')
						->fields(array('order_status' => 'filing_printed'))
						->condition('order_id', $orderid, '=')
						->execute();
				uc_order_comment_save($product->order_id, $user->uid, 'Printed Articles of Organization for '.$company_name,'order');
				$result = db_update('uc_orders')
						->fields(array('order_status' => 'filing_printed'))
						->condition('order_id', $product->order_id, '=')
						->execute();
				include_once('tbs_class.php');
				include_once('tbs_plugin_opentbs.php');
			
				$TBS = new clsTinyButStrong;
				$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
			
				$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name); // Load the archive 'document.odt'.
				$TBS->MergeBlock('a', $data);
				$TBS->Show(OPENTBS_DOWNLOAD, 'Articles-'.$company_name.'.odt');  
				break; 				
	}//switch
	drupal_set_message('done');
}
function wysos_workflow_resolutions($order_product_id){
	if(! is_numeric($order_product_id)){
		return drupal_access_denied();
	}
	global $user;
	
	//Get Order Product
	$product = db_query("SELECT * FROM uc_order_products WHERE order_product_id = :product_id",array(':product_id'=> $order_product_id))->fetchObject();
	if(!$product){
		return drupal_access_denied();
	}
	$data = unserialize($product->data);
	
	//Get order UID
	$orderid = $product->order_id;
	$uid = db_query("SELECT uid FROM uc_orders WHERE order_id = :orderid", array(':orderid'=>$orderid));
	$order_user = user_load($uid);
	
	// Set value based on order type 
	switch($product->model){
		case 'CLOSE' :
		case 'WYCLOSE' :
		case 'WYP1R' :
		case 'WYP' :
			//Profit Corporation
			if($data['attributes']['Nominee'][0] == '(none)' || empty($data['attributes']['Nominee'][0])){
				$doc_name = 'Resolutions-PROFITCORP.odt';
			}
			else{
				$doc_name = 'Resolutions-PROFITCORP-'.$data['attributes']['Nominee'][0].'.odt';
			}
			$company_name = $data['attributes']['Name of the company'][0];
			$GLOBALS['company_name'] = $company_name;
			$GLOBALS['username'] = $order_user->name;
			$GLOBALS['password'] = $order_user->name.'11';
			$GLOBALS['director_1_name'] = $data['attributes']['Director #1 Name'][0];
			$GLOBALS['director_1_address'] = $data['attributes']['Director #1 Address'][0];
			$data2 = array();
			$data[] = array('name' => $data['attributes']['Director #1 Name'][0],'address' => $data['attributes']['Director #1 Address']);
			if(!empty($data['attributes']['Director #2 Name'][0])){
				$data2[] = array('name' => $data['attributes']['Director #2 Name'][0], 'address'=>$data['attributes']['Director #2 Address'][0]);
				$GLOBALS['director_2_name'] = $data['attributes']['Director #2 Name'][0];
				$GLOBALS['director_2_address'] = $data['attributes']['Director #2 Address'][0];
			}
			if(!empty($data['attributes']['Director #3 Name'][0])){
				$data2[] = array('name' => $data['attributes']['Director #3 Name'][0], 'address'=>$data['attributes']['Director #3 Address'][0]);
				$GLOBALS['director_3_name'] = $data['attributes']['Director #3 Name'][0];
				$GLOBALS['diretor_3_address'] = $data['attributes']['Director #3 Address'][0];
			}
			if(!empty($data['attributes']['Director #4 Name'][0])){
				$data2[] = array('name'=>$data['attributes']['Director #4 Name'][0], 'address'=>$data['attributes']['Director #4 Address'][0]);
				$GLOBALS['director_4_name'] = $data['attributes']['Director #4 Name'][0];
				$GLOBALS['director_4_address'] = $data['attributes']['Director #4 Address'][0];
			}	
			//Set Order Status
			
			if(in_array('Options', $data['attributes'])){
				if(in_array('Corporate Record Book, Seal, Stock Certificates and other Forms',$data['attributes']['Options'])){
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_record_book'))
						->condition('order_id',$orderid,'=')
						->execute();
				}
				elseif(in_array('Initial Minutes and Bylaws/Operating Agreement',$data['attributes']['Options'])){
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_minutes_print'))
						->condition('order_id',$orderid,'=')
						->execute();
				}
				elseif($data['attributes']['Virtual Office Term'][0] != '(none)' && !empty($value->data['attributes']['Virtual Office Term'][0])){
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_vo_configure'))
						->condition('order_id',$orderid,'=')
						->execute();
				}
				else{
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_mailing'))
						->condition('order_id',$orderid,'=')
						->execute();
				}
			
			}else{
				$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_mailing'))
						->condition('order_id',$orderid,'=')
						->execute();
			}
			uc_order_comment_save($orderid, $user->uid, 'Printed Resolutions for '.$company_name,'order');
			
			include_once('tbs_class.php');
			include_once('tbs_plugin_opentbs.php');
			
			$TBS = new clsTinyButStrong;
			$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
			$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name);
			
			$TBS->MergeBlock('a', $data2);
			$TBS->Show(OPENTBS_DOWNLOAD, 'Resolutions-'.$company_name.'.odt'); 
			break;
		case 'CLLC' :
		case 'WYLLC' :
		case 'WYLLC1R' :
			//LLC
			if($data['attributes']['Nominee'][0] == '(none)' || empty($data['attributes']['Nominee'][0]))
				$doc_name = 'Resolutions-LLC.odt';
			else
				$doc_name = 'Resolutions-LLC-'.$data['attributes']['Nominee'][0].'.odt';
			$company_name = $data['attributes']['Name of the company'][0];
			if(stripos($company_name,' LLC')===false && stripos($company_name,' LC')===false && stripos($company_name,' Limited Liability Company')===false 
					&& stripos($company_name,' Ltd. Liability Company')===false && stripos($company_name,' L.L.C.')===false 
					&& stripos($company_name,' Limited Company')===false) {
				$company_name = trim($company_name).' LLC';
			}
			$GLOBALS['company_name'] = $company_name;
			$GLOBALS['username'] = $order_user->name;
			$GLOBALS['password'] = $order_user->name.'11';
			$GLOBALS['manager_1_name'] = $data['attributes']['Manager #1 Name'][0];
			$GLOBALS['manager_1_address'] = $data['attributes']['Manager #1 Address'][0];
			$data2 = array();
			$data[] = array('name'=>$data['attributes']['Manager #1 Name'][0], 'address'=>$data['attributes']['Manager #1 Address'][0]);
			if(!empty($data['attributes']['Manager #2 Name'][0])){
				$data2[] = array('name'=>$data['attributes']['Manager #2 Name'][0], 'address'=>$data['attributes']['Manager #2 Address'][0]);
				$GLOBALS['manager_2_name'] = $data['attributes']['Manager #2 Name'][0];
				$GLOBALS['manager_2_address'] = $data['attributes']['Manager #2 Address'][0];
			}
			if(!empty($data['attributes']['Manager #3 Name'][0])){
				$data2[] = array('name'=>$data['attributes']['Manager #3 Name'][0], 'address'=>$data['attributes']['Manager #3 Address'][0]);
				$GLOBALS['manager_3_name'] = $data['attributes']['Manager #3 Name'][0];
				$GLOBALS['manager_3_address'] = $data['attributes']['Manager #3 Address'][0];
			}
			if(!empty($data['attributes']['Manager #4 Name'][0])){
				$data2[] = array('name'=>$data['attributes']['Manager #4 Name'][0], 'address'=>$data['attributes']['Manager #4 Address'][0]);
				$GLOBALS['manager_4_name'] = $data['attributes']['Manager #4 Name'][0];
				$GLOBALS['manager_4_address'] = $data['attributes']['Manager #4 Address'][0];
			}
			if(in_array('Options', $data['attributes'])) {
				if(in_array('Corporate Record Book, Seal, Stock Certificates and other Forms',$data['attributes']['Options'])){
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_record_book'))
						->condition('order_id',$orderid,'=')
						->execute();
		
				}
				elseif(in_array('Initial Minutes and Bylaws/Operating Agreement',$data['attributes']['Options'])){
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_minutes_print'))
						->condition('order_id',$orderid,'=')
						->execute();
				}
				elseif($data['attributes']['Virtual Office Term'][0] != '(none)' && !empty($value->data['attributes']['Virtual Office Term'][0])){
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_vo_configure'))
						->condition('order_id',$orderid,'=')
						->execute();
				}
				else{
					$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_mailing'))
						->condition('order_id',$orderid,'=')
						->execute();
				}
			}
			else{
				$result = db_update('uc_orders')
						->fields(array('order_status'=>'wait_mailing'))
						->condition('order_id',$orderid,'=')
						->execute();
			}
			uc_order_comment_save($orderid, $user->uid, 'Printed Resolutions for '.$company_name,'order');

			include_once('tbs_class.php');
			include_once('tbs_plugin_opentbs.php');
		
			$TBS = new clsTinyButStrong;
			$TBS->Plugin(TBS_INSTALL, OPENTBS_PLUGIN);
		
			$TBS->LoadTemplate(filesystem_base_path().'/sites/all/modules/wysos/'.$doc_name); // Load the archive 'document.odt'.
			$TBS->MergeBlock('a', $data2);
			$TBS->Show(OPENTBS_DOWNLOAD, 'Resolutions-'.$company_name.'.odt');  
			break;
	}
	drupal_goto('/wysos/workflow');
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


















