<?php
/**
 *  The clips extension engine and execution context
 */
class Clips {

	/**
	 * The clips execution context
	 */
	private $context;

	private function __construct() {
		$this->context = array();
		clips_init($this->context);
	}

	public function __get($key) {
		if(isset($this->context[$key])) {
			return $this->context[$key];
		}
		return $this->$key;
	}

	public function __set($key, $value) {
		$this->context[$key] = $value;
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

	/**
	 * Get the clips instance
	 */
	public static function GetInstance() {
		static $instance = null;
		if(null === $instance) {
			$instance = new Clips();
		}
		return $instance;
	}
}
