# php-clips: The [CLIPS](http://clipsrules.sourceforge.net) execution extension for [PHP 5 and later](http://php.net)

## Introduction

This little plugin is a extension for embed the wonderful rule engine [CLIPS](http://clipsrules.sourceforge.net) into PHP.

Before this extension, there is another PHP extension about CLIPS with a nice name [phlips](http://phlips.sourceforge.net).

So, why I need to create another PHP extension?

1. phlips is long dead, the last active news of this project is at date January 19, 2005.
2. phlips is not a well designed framework to integrate CLIPS into PHP's environment, it is just a wrapper for all the CLIPS APIs

So, I choose to write a PHP extension on my own.

## What is CLIPS

CLIPS is a forward-chaining rule-based programming language written in C that also provides procedural and object-oriented programming facilities.

## Why need to embed CLIPS into PHP

1. For most rule based logic, it is easy and fast to write rules and execute them in the rule engine than to write very complex codes
2. The rules a much more easier to customize than the code
3. CLIPS lacks many useful functions(for example the regex support, can't connect to any datasource), but can get this from PHP
4. CLIPS is very very fast, here is the result of how fast it should be:

The code to calculate fibo 100 

	(defglobal ?*count* = 100)
	(defrule fibo
		?n <- (n ?x ?xv)
		(n ?y&:(and (= (+ ?x 1) ?y) (<= ?y ?*count*)) ?yv)
			=>
		(retract ?n)
		(assert (n (+ ?y 1) (+ ?xv ?yv))))
	(defrule output
		(n ?y&:(= ?y ?*count*) ?v)
			=>
		(printout t "The fibo of " ?y " is " ?v crlf))
	(assert (n 1 1))
	(assert (n 2 1))
	(run)

and the result:

	The fibo of 100 is 3736710778780434371
	pclips fibo.rules  0.03s user 0.01s system 89% cpu 0.046 total

For it'll take about 0.04s to initliaze the PHP and the php-clips extension.

	PHP 5.6.4 (cli) (built: Dec 23 2014 11:42:26) (DEBUG)
	Copyright (c) 1997-2014 The PHP Group
	Zend Engine v2.6.0, Copyright (c) 1998-2014 Zend Technologies
		with Xdebug v2.2.6, Copyright (c) 2002-2014, by Derick Rethans
		php -v  0.02s user 0.01s system 82% cpu 0.030 total

## Features

### Fully integrate CLIPS v6.3 into PHP

The CLIPS code is distributed with this code, and will be compile into the extension, so you need nothing else to installed.

### Can call PHP's function in CLIPS's execution environment

You can just call php's method(all in the current run context) in clips like this:

	(php_call "print_r" (create$ 1 2 "3" Four))

And get result:

	Array
	(
		[0] => 1
		[1] => 2
		[2] => 3
		[3] => Four
	)

### Can translate PHP's array or objects into facts(with templates)

This is the main feature of php-clips, you can do this in php-clips:

	class Dummy { 
		/** @ClipsMulti */
		public $args;
		public $num;

		public function __construct($args) {
			$this->args = $args;
			$this->num = count(num);
		}
	}

	$clips->assertFacts(new Dummy(array('hello', 'world')));
	
and get the fact of:

	(Dummy (args "hello" "world") (num 2))

The annotation support is based on the wonderful php annotation library [addendum](https://code.google.com/p/addendum/).

### Can access the PHP object as the reference of the CLIPS object

php-clips also provied a context to let CLIPS access PHP's object directly by using CLIPS's object name;

Here is the example(the array is translated into multiple fields automaticly):
	$clips->dummy = new Dummy(array('hello', 'world'));

	(printout t "The args is " (php_property [dummy] "args"))

And you will get
	
	"hello" "world"

## Installation

The installation of this plugin is very easy, just like other php extensions:

1. git clone https://github.com/guitarpoet/php-clips.git
2. cd php-clips
3. phpize
4. ./configure
5. make && make install

Feel free to play with it, and contact me if you found a bug or have any intresting ideas.
