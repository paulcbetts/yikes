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

# Standard library
require 'rubygems'
require 'logger'
require 'gettext'
require 'singleton'
require 'fileutils'
require 'pathname'
require 'yaml'

# Yikes
require 'platform'
require 'utility'
require 'state'

include GetText

$logging_level ||= Logger::ERROR

class Engine 
	include ApplicationState

	def initialize(transcoder = FFMpegTranscoder, library = nil)
		@transcoder = transcoder.new
		new_state library if library
	end

	def enqueue_files_to_encode(library, target)
		fl = get_file_list(library).delete_if{|x| not AllowedFiletypes.include?(Pathname.new(x).extname.downcase)}
		state.add_to_queue(fl.collect {|x| EncodingItem.new(library, x, target)})
	end

	def do_encode(library, target)
		logger.info "Collecting files.."
		enqueue_files_to_encode(library, target)

		logger.info "Starting encoding run.."
	 	state.dequeue_items(state.items_count).each do |item|
			unless should_encode?(item)
				logger.debug "Already exists: #{item.source_path}"
				next
			end

			logger.debug "Trying '#{item.source_path}'"
			convert_file(item, state)
		end

		logger.info "Finished"
	end

	# Main function for converting a video file and writing it to a folder
	def convert_file(item, state)
		if create_path_and_convert(item)
			logger.debug "Convert succeeded"
			state.encode_succeeded!(item)
		else
			logger.debug "Convert failed"
			state.encode_failed!(item)
		end
	end

	def create_path_and_convert(item)
		dest_file = Pathname.new(item.target_path)
		FileUtils.mkdir_p(dest_file.dirname.to_s)

		ret = transcode(item.source_path, dest_file.to_s)
		get_screenshot(item.source_path, item.screenshot_path) if ret
		ret
	end

	def transcode(input, output)
		p = Pathname.new(output)
		if p.exist?
			logger.info "#{p.basename.to_s} already exists, skipping"
			return true
		end
		logger.info "#{p.basename.to_s} doesn't exist, starting transcode..."

		@transcoder.transcode(input, output)
	end

	def get_screenshot(input, output)
		@transcoder.before_transcode
		logger.debug "Saving screenshot to #{output}"
		@transcoder.get_screenshot(input, output)
	end

	def should_encode?(item)
		p = Pathname.new(item.target_path)
		return true unless p.exist?
		return p.size <= 256
	end
end

module EngineManager
	def add_engine(library, target, transcoder = FFMpegTranscoder)
		initialize_if_needed

		e = nil
		@engines_lock.synchronize do
			e = (@engines[engine_key(library, target)] ||= Engine.new(transcoder, library))
		end
		e
	end

	def active_engines
		initialize_if_needed

		ret = nil
		@engines_lock.synchronize do
			ret = @engines.values
		end
		ret
	end

	def remove_engine(library, target)
		initialize_if_needed

		@engines_lock.synchronize do
			@engines.delete engine_key(library, target)
		end
	end

	def load_state(path)
		initialize_if_needed

		begin 
			# We hold a different State class for every library path
			@engines = YAML::load(File.read(path))
		rescue
			@engines = {}
		end
	end

	def save_state(path)
		initialize_if_needed

		@engines_lock.synchronize do
			File.open(path, 'w') {|f| f.write(YAML::dump(@engines)) }
		end
	end

	def poll_directory_and_encode(rate)
		until $do_quit
			# FIXME: This is unsynchronized, but if we use the lock, we'll block everyone 
			# else forever
			@engines.each_pair do |key,engine|
				break if $do_quit
				library, target = *key

				logger.info "Starting encode for '#{library}' => '#{target}'"
				engine.do_encode(library,target)
				Kernel.sleep(rate || 60*30)
			end

			while @engines.length == 0 
				break if $do_quit
				logger.info _("Nothing to do! Taking a nap...")
				Kernel.sleep(rate || 60*30)
			end
		end
	end

private
	def initialize_if_needed
		@engines ||= {}
		@engines_lock ||= Mutex.new
	end

	def engine_key(library, target)
		[library, target]
	end
end

module ExternalTranscoder
	def transcode(input, output)
		before_transcode
		cmd = get_transcode_command(input, output)
		Platform.run_external_command(cmd)
	end

	def get_screenshot(input, output)
		before_transcode
		cmd = get_screenshot_command(input, output)
		Platform.run_external_command(cmd)
	end

private

end

class FFMpegTranscoder
	include ExternalTranscoder

	# Encoding notes:
	# 	Getting QuickTime to read the videos we generate is a *giant* pain in the
	# 	ass; it barfs at anything that's nonstandard and I've spent lots of time
	# 	trying to find the ffmpeg "magic words" that make it happen. The relevant
	# 	parts seem to be:
	#
	# 	Framerate MUST be NTSC i.e. 30000/1001
	# 	vcodec MUST be either xvid or libx264, acodec MUST be aac
	#
	# 	Versions of ffmpeg call we've tried:
	#
	#	# One-pass known-working with QT
	#	$FFMPEG -y -i "$1" -f mov -vcodec libx264 -s 480x320 -r 30000/1001 -bufsize 8192 -qscale 5 -acodec libfaac -ab 96 -ac 2 -async 1 -threads auto "$2"
	#
	# 	# Revised 2-pass code
	#	$FFMPEG -y -threads auto -i "$1" -f mov -vcodec libx264 -s 432x320 -r 30000/1001 -bufsize 8192 -qscale 5
	#	-refs 1 -loop 1 -deblockalpha 0 -deblockbeta 0 -parti4x4 1 -partp8x8 1 -me full -subq 1 -me_range 21 \
	#	-chroma 1 -slice 2 -bf 0 -level 30 -g 300 -keyint_min 30 -sc_threshold 40 -rc_eq 'blurCplx^(1-qComp)' \
	#	-qcomp 0.7 -qmax 51 -qdiff 4 -i_qfactor 0.71428572 -maxrate 768000 -cmp 1 -s 480x320 -f mov -pass 1 \
	#	/dev/null
	#
	#	$FFMPEG -y -i "$1" -v 1 -threads auto -vcodec libx264 -r 30000/1001 -bufsize 8192 -qscale 5 -refs 1 \
	#	-loop 1 -deblockalpha 0 -deblockbeta 0 -parti4x4 1 -partp8x8 1 -me full -subq 6 -me_range 21 -chroma 1 \
	#	-slice 2 -bf 0 -level 30 -g 300 -keyint_min 30 -sc_threshold 40 -rc_eq 'blurCplx^(1-qComp)' -qcomp 0.7 \
	#	-qmax 51 -qdiff 4 -i_qfactor 0.71428572 -maxrate 768000 -cmp 1 -s 480x320 -acodec libfaac -ab 96 -ar \
	#	48000 -ac 2 -f mov -pass 2 "$2"
	#
	#	# Original 2-pass code
	#	# $FFMPEG -y -i "$1" -an -v 1 -threads auto -vcodec libx264 -b 500000 -bt 175000 \
	#	-refs 1 -loop 1 -deblockalpha 0 -deblockbeta 0 -parti4x4 1 -partp8x8 1 -me full -subq 1 -me_range 21 \
	#	-chroma 1 -slice 2 -bf 0 -level 30 -g 300 -keyint_min 30 -sc_threshold 40 -rc_eq 'blurCplx^(1-qComp)' \
	#	-qcomp 0.7 -qmax 51 -qdiff 4 -i_qfactor 0.71428572 -maxrate 768000 -bufsize 2M -cmp 1 -s 480x320 -f mp4\
	#	-pass 1 /dev/null
	#
	#	$FFMPEG -y -i "$1" -v 1 -threads auto -vcodec libx264 -b 500000 -bt 175000 -refs 1 -loop 1 -deblockalpha\
	#	0 -deblockbeta 0 -parti4x4 1 -partp8x8 1 -me full -subq 6 -me_range 21 -chroma 1 -slice 2 -bf 0 -level \
	#	30 -g 300 -keyint_min 30 -sc_threshold 40 -rc_eq 'blurCplx^(1-qComp)' -qcomp 0.7 -qmax 51 -qdiff 4 \
	#	-i_qfactor 0.71428572 -maxrate 768000 -bufsize 2M -cmp 1 -s 480x320 -acodec libfaac -ab 96 -ar 48000 -ac\
	#	2 -f mp4 -pass 2 "$2"

	MiscParams = %w(-y -threads auto -maxrate 3584000 -bufsize 1536000 -async 1)

	VideoParams = %w(-vcodec libx264 -r 30000/1001 -b 1536000 -level 30 -mbd 2 -cmp 2 -subcmp 2 -g 300 -qmin 20
	-qmax 31 -flags +loop -chroma 1 -partitions partp8x8+partb8x8 -flags2 +mixed_refs -me_method 8 -subq 7
	-trellis 2 -refs 1 -coder 0 -me_range 16 -bf 0 -sc_threshold 40 -keyint_min 25 -s 488x328
	-croptop 4 -cropbottom 4 -cropleft 4 -cropright 4 -aspect 4:3 )

	AudioParams = %w( -acodec libfaac -ac 2 -ar 44100 -ab 128000 -aq 120 -alang ENG )

	FFMpegPath = File.join(AppConfig::LibDir, 'libexec', 'bin', 'ffmpeg')
	def get_transcode_command(input, output)
		ret = ["#{FFMpegPath} -i \"#{input}\""] + MiscParams + VideoParams + AudioParams + ["\"#{output}\""]
		ret.join ' '
	end

	ScreenshotParams = %w( -f mjpeg -s 244x164 -vframes 1 -ss 2 )
	def get_screenshot_command(input, output)
		ret = ["#{FFMpegPath} -i \"#{input}\""] + ScreenshotParams + ["\"#{output}\""]
		ret.join ' '
	end

	def before_transcode
		libpath = File.join(AppConfig::LibDir, 'libexec', 'lib')

		logger.debug "Setting LD_LIBRARY_PATH to '#{libpath}'..."
		ENV["LD_LIBRARY_PATH"] = libpath
		ENV["DYLD_LIBRARY_PATH"] = libpath
	end
end
