<?php
	clips_init(array());
	$arr = array();
	clips_create_env("test");

	clips_meta($arr);
	print_r($arr);
	clips_load(dirname(__FILE__).'/test.rules');

	clips_switch_env("test");
	clips_exec('(facts)');

	clips_switch_env("MAIN");
	clips_exec('(facts)');
