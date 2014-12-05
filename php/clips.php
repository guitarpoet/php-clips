<?php

function clips_get_property($obj, $property) {
	if(is_array($obj) && isset($obj[$property])) {
		return $obj[$property];
	}

	if(is_object($obj) && isset($obj->$property)) {
		return $obj->$property;
	}
	return null;
}
/**
 *  The clips extension engine and execution context
 */
class Clips {

	/**
	 * The clips execution context
	 */
	private static $context;

	public function __construct() {
		if(!Clips::$context) {
			Clips::$context = array();
			clips_init(Clips::$context);
		}
	}

	public function __get($key) {
		if(isset(Clips::$context[$key])) {
			return Clips::$context[$key];
		}
		return $this->$key;
	}

	public function __set($key, $value) {
		Clips::$context[$key] = $value;
	}

	/**
	 * Start the clips in console mode
	 */
	public function console() {
		$line = readline('pclips$ ');
		$line .= "\n";
		while(true) {
			if(clips_is_command_complete($line)) {
				$this->command($line);
				readline_add_history($line);
				$line = readline('pclips$ ');
			}
			else {
				$line .= readline('... ');
			}
		}
	}

	public function instanceExists($name) {
		if($name)
			return clips_instance_exists($name);
		return false;
	}

	public function templateExists($template) {
		if($template)
			return clips_template_exists($template);
		return false;
	}

	/**
	 * Execute the clips command
	 */
	public function command($command) {
		if(is_array($command)) {
			foreach($command as $c) {
				$this->command($c);
			}
			return;
		}
		clips_exec($command);
	}

	/**
	 * Load and execute the clips rule file
	 */
	public function load($file) {
		if(is_array($file)) {
			foreach($file as $f) {
				$this->load($f);
			}
			return;
		}

		if(file_exists($file)) {
			clips_load($file);
		}
		else {
			trigger_error('The file '.$file.' is not found.');
		}
	}
}
