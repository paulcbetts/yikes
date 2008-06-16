###########################################################################
#   Copyright (C) 2008 by Paul Betts                                      #
#   paul.betts@gmail.com                                                  #
#                                                                         #
#   This program is free software; you can redistribute it and/or modify  #
#   it under the terms of the GNU General Public License as published by  #
#   the Free Software Foundation; either version 2 of the License, or     #
#   (at your option) any later version.                                   #
#                                                                         #
#   This program is distributed in the hope that it will be useful,       #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of        #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         #
#   GNU General Public License for more details.                          #
#                                                                         #
#   You should have received a copy of the GNU General Public License     #
#   along with this program; if not, write to the                         #
#   Free Software Foundation, Inc.,                                       #
#   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             #
###########################################################################

require File.join(File.dirname(__FILE__), 'config')
path_add File.join(File.dirname(__FILE__))

# FIXME: We probably need this because of the path_filter code not preserving the order
# This is to stop us from including the local version of RubyGems
$:.unshift File.dirname(__FILE__)

# Standard library
require 'rubygems'
require 'logger'
require 'gettext'
require 'optparse'
require 'optparse/time'
require 'singleton'
require 'yaml'
require 'ramaze'

# Yikes
require 'platform'
require 'utility'
require 'engine'
require 'daemonize'
require 'state'
require 'controllers'

include GetText

$logging_level = ($DEBUG ? Logger::DEBUG : Logger::ERROR)

AllowedFiletypes = ['.avi', '.mov', '.mp4', '.wmv']

class Yikes < Logger::Application
	include Singleton
	attr_accessor :engine

	def initialize
		super(self.class.to_s) 
		self.level = $logging_level
	end

	#
	# Main Methods
	#

	def parse(args)
		# Set the defaults here
		results = { :target => 'iPod', :library => '.' }

		opts = OptionParser.new do |opts|
			opts.banner = _("Usage: Yikes [options]")

			opts.separator ""
			opts.separator _("Specific options:")

			opts.on('-l', _("--library /path/to/videos"),
				_("Directory to recursively scan for videos")) do |x|
				results[:library] = Pathname.new(x).realpath.to_s
			end

			opts.on('-t', "--target dir", _("Directory to put files in")) do |x|
				results[:target] = Pathname.new(x).realpath.to_s
			end

			opts.on('-b', '--background', _("Run this program in the background")) do
				results[:background] = true
			end

			opts.on('-r', '--rate seconds', _("How long to wait between runs; implies -b")) do |x|
				results[:background] = true
				results[:rate] = x.to_s.to_f
			end

			opts.separator ""
			opts.separator _("Common options:")

			opts.on_tail("-?", "--help", _("Show this message") ) do
				puts opts
				return nil
			end

			opts.on('-d', "--debug", _("Run in debug mode (Extra messages)")) do |x|
				$logging_level = DEBUG
			end

			opts.on('-v', "--verbose", _("Run verbosely")) do |x|
				$logging_level = INFO 
			end

			opts.on_tail("--version", _("Show version") ) do
				puts OptionParser::Version.join('.')
				return nil	
			end
		end

		opts.parse!(args)
		results
	end

	def run(args)
		# Initialize Gettext (root, domain, locale dir, encoding) and set up logging
    		bindtextdomain(AppConfig::Package, nil, nil, "UTF-8")
		@log.level = Logger::DEBUG
		
		# Parse arguments
		begin
			results = parse(args)
		rescue OptionParser::MissingArgument
			puts _('Missing parameter; see --help for more info')
			return -1	
		rescue OptionParser::InvalidOption
			puts _('Invalid option; see --help for more info')
			return -1	
		end

		return -1 unless results

		# Reset our logging level because option parsing changed it
		@log.level = $logging_level

		@engine = Engine.new
		@engine.load_state(File.join(Platform.settings_dir, 'state.yaml'), results[:library])
		@engine.state.target = results[:target]

		# Actually do stuff
		unless results[:background]
			# Just a single run
			@engine.do_encode(results[:library], results[:target])
		else
			puts _("Yikes started in the background. Go to http://#{Platform.hostname}.local:4000 !")
			if should_daemonize?
				return unless daemonize 
				@log = Logger.new('/tmp/yikes.log')
			end

			start_web_service_async

			begin 
				@engine.poll_directory_and_encode(results[:library], results[:target], results[:rate])
			rescue Exception => e
				logger.fatal e.message
				logger.fatal e.backtrace.join("\n")
			end
		end

		@engine.save_state(File.join(Platform.settings_dir, 'state.yaml'))

		logger.debug 'Exiting application'
	end



	#
	# Auxillary methods
	#

	def should_daemonize?
		(not $logging_level == DEBUG)
	end

	def get_logger; @log; end

	def start_web_service_async
		Thread.new do
			Ramaze.start :adaptor => :webrick, :port => 4000
		end
	end

	def self.url_base
		#"http://#{Platform.hostname}.local:4000"
		"http://localhost:4000"
	end
end

def logger
	Yikes.instance.get_logger
end

if __FILE__ == $0
	$the_app = Yikes.instance
	exit ($the_app.run(ARGV))
end
