# encoding: ascii-8bit

# Copyright 2022 Ball Aerospace & Technologies Corp.
# All Rights Reserved.
#
# This program is free software; you can modify and/or redistribute it
# under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation; version 3 with
# attribution addendums as found in the LICENSE.txt
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# This program may also be used under the terms of a commercial or
# enterprise edition license of COSMOS if purchased from the
# copyright holder

# Redefine Object.load so simplecov doesn't overwrite the results after
# re-loading a file during test.
# def load(file, wrap = false)
#   if defined? SimpleCov
#     SimpleCov.command_name "#{file}#{(Time.now.to_f * 1000).to_i}"
#   end
#   Kernel.load(file, wrap)
# end

# NOTE: You MUST require simplecov before anything else!
if !ENV['COSMOS_NO_SIMPLECOV']
  require 'simplecov'
  if ENV['GITHUB_WORKFLOW']
    require 'codecov'
    SimpleCov.formatter =
      SimpleCov::Formatter::MultiFormatter.new(
        [SimpleCov::Formatter::HTMLFormatter, SimpleCov::Formatter::Codecov],
      )
  end
  SimpleCov.start do
    merge_timeout 60 * 60 # merge the last hour of results
    add_filter '/spec/' # no coverage on spec files
    root = File.dirname(__FILE__)
    root.to_s
  end
  SimpleCov.at_exit do
    Cosmos.disable_warnings do
      Encoding.default_external = Encoding::UTF_8
      Encoding.default_internal = nil
    end
    SimpleCov.result.format!
  end
end
require 'rspec'

# Disable Redis and Fluentd in the Logger
ENV['COSMOS_NO_STORE'] = 'true'
# Set some passwords
ENV['COSMOS_API_PASSWORD'] = 'cosmos'
# Set internal cosmos password
ENV['COSMOS_SERVICE_PASSWORD'] = 'cosmosservice'
# Set redis username
ENV['COSMOS_REDIS_USERNAME'] = 'cosmos'
# Set redis password
ENV['COSMOS_REDIS_PASSWORD'] = 'cosmospassword'
# Set minio password
ENV['COSMOS_MINIO_USERNAME'] = 'cosmosminio'
# Set minio password
ENV['COSMOS_MINIO_PASSWORD'] = 'cosmosminiopassword'
# Set cosmos scope
ENV['COSMOS_SCOPE'] = 'DEFAULT'

module Cosmos
  USERPATH = File.join(File.dirname(File.expand_path(__FILE__)), 'install')
end

require 'cosmos/top_level'
require 'cosmos/script'
# require 'cosmos/utilities/logger'
# Create a easy alias to the base of the spec directory
SPEC_DIR = File.dirname(__FILE__)
$cosmos_scope = ENV['COSMOS_SCOPE']
$cosmos_token = ENV['COSMOS_API_PASSWORD']
$cosmos_authorize = false

def setup_system(targets = %w[SYSTEM INST EMPTY])
  result = nil
  capture_io do |stdout|
    require 'cosmos/system'
    dir = File.join(__dir__, 'install', 'config', 'targets')
    Cosmos::System.class_variable_set(:@@instance, nil)
    Cosmos::System.instance(targets, dir)
    result = stdout
  end
  result
end

def get_all_redis_keys
  cursor = 0
  keys = []
  loop do
    cursor, result = Cosmos::Store.scan(cursor)
    keys.concat(result)
    cursor = cursor.to_i # cursor is returned as a string
    break if cursor == 0
  end
  keys
end

Cosmos.disable_warnings do
  require 'redis'
  require 'mock_redis'
  class MockRedis
    module StreamMethods
      private

      def with_stream_at(key, &blk)
        @mutex ||= Mutex.new
        @mutex.synchronize do
          with_thing_at(key, :assert_streamy, proc { Stream.new }, &blk)
        end
      end
    end
  end
end

def mock_redis
  redis = MockRedis.new
  allow(Redis).to receive(:new).and_return(redis)

  # pool = double(ConnectionPool)
  # allow(pool).to receive(:with) { redis }
  # allow(ConnectionPool).to receive(:new).and_return(pool)
  Cosmos::Store.instance_variable_set(:@instance, nil)
  Cosmos::EphemeralStore.instance_variable_set(:@instance, nil)
  require 'cosmos/models/auth_model'
  Cosmos::AuthModel.set($cosmos_token, nil)
  redis
end

# Clean up the spec configuration directory
def clean_config
  %w[
    outputs/logs
    outputs/saved_config
    outputs/tmp
    outputs/tables
    outputs/handbooks
    procedures
  ].each do |dir|
    FileUtils.rm_rf(Dir.glob(File.join(Cosmos::USERPATH, dir, '*')))
  end
end

# Set the logger to output everthing and capture it all in a StringIO object
# which is yielded back to the block. Then restore everything.
def capture_io(output = false)
  # Set the logger level to DEBUG so we see all output
  Cosmos::Logger.instance.level = Logger::DEBUG

  # Create a StringIO object to capture the output
  stdout = StringIO.new('', 'r+')
  $stdout = stdout
  saved_stdout = nil
  Cosmos.disable_warnings do
    # Save the old STDOUT constant value
    saved_stdout = Object.const_get(:STDOUT)

    # Set STDOUT to our StringIO object
    Object.const_set(:STDOUT, $stdout)
  end

  # Yield back the StringIO so they can match against it
  yield stdout

  # Restore the logger to FATAL to prevent all kinds of output
  Cosmos::Logger.level = Logger::FATAL

  # Restore the STDOUT constant
  Cosmos.disable_warnings { Object.const_set(:STDOUT, saved_stdout) }

  # Restore the $stdout global to be STDOUT
  $stdout = STDOUT
  puts stdout.string if output # Print the capture for debugging
end

# Get a list of running threads, ignoring jruby system threads if necessary.
def running_threads
  threads = []
  Thread.list.each do |t|
    if RUBY_ENGINE == 'jruby'
      thread_name = JRuby.reference(t).native_thread.get_name
      unless thread_name == 'Finalizer' or thread_name.include?('JRubyWorker')
        threads << t.inspect
      end
    else
      threads << t.inspect
    end
  end
  return threads
end

# Kill threads that are not "main", ignoring jruby system threads if necessary.
def kill_leftover_threads
  if RUBY_ENGINE == 'jruby'
    if Thread.list.length > 2
      Thread.list.each do |t|
        thread_name = JRuby.reference(t).native_thread.get_name
        if t != Thread.current and thread_name != 'Finalizer' and
             !thread_name.include?('JRubyWorker')
          t.kill
        end
      end
      sleep(0.2)
    end
  else
    if Thread.list.length > 1
      Thread.list.each { |t| t.kill if t != Thread.current }
      sleep(0.2)
    end
  end
end

$system_exit_count = 0
# Overload exit so we know when it is called
alias old_exit exit
def exit(*args)
  $system_exit_count += 1
end

RSpec.configure do |config|
  # Enforce the new expect() syntax instead of the old should syntax
  config.expect_with :rspec do |c|
    c.syntax = :expect
    c.max_formatted_output_length = nil # Prevent RSpec from doing truncation
  end

  # Store standard output global and CONSTANT since we will mess with them
  config.before(:all) do
    $saved_stdout_global = $stdout
    $saved_stdout_const = Object.const_get(:STDOUT)
  end

  config.after(:all) do
    Cosmos.disable_warnings do
      def Object.exit(*args)
        old_exit(*args)
      end
    end
  end

  # Before each test make sure $stdout and STDOUT are set. They might be messed
  # up if a spec fails in the middle of capture_io and we don't have a chance
  # to return and reset them.
  config.before(:each) do
    $stdout = $saved_stdout_global if $stdout != $saved_stdout_global
    Cosmos.disable_warnings { Object.const_set(:STDOUT, $saved_stdout_const) }
    kill_leftover_threads
  end

  config.after(:each) do
    # Make sure we didn't leave any lingering threads
    threads = running_threads
    thread_count = threads.size
    running_threads_str = threads.join("\n")

    expect(thread_count).to eql(1),
    "At end of test expect 1 remaining thread but found #{thread_count}.\nEnsure you kill all spawned threads before the test finishes.\nThreads:\n#{running_threads_str}"
  end
end

# Commented out for performance reasons
# If you want to manually profile, benchmark, or stress test then uncomment
# require 'ruby-prof' if RUBY_ENGINE == 'ruby'
# require 'benchmark/ips' if ENV.key?("BENCHMARK")
# RSpec.configure do |c|
#   if ENV.key?("PROFILE")
#     c.before(:suite) do
#       RubyProf.start
#     end
#     c.after(:suite) do |example|
#       result = RubyProf.stop
#       result.exclude_common_methods!
#       printer = RubyProf::GraphHtmlPrinter.new(result)
#       printer.print(File.open("profile.html", 'w+'), :min_percent => 1)
#     end
#     c.around(:each) do |example|
#       # Run each test 100 times to prevent startup issues from dominating
#       100.times do
#         example.run
#       end
#     end
#   end
#   if ENV.key?("BENCHMARK")
#     c.around(:each) do |example|
#       Benchmark.ips do |x|
#         x.report(example.metadata[:full_description]) do
#           example.run
#         end
#       end
#     end
#   end
#   if ENV.key?("STRESS")
#     c.around(:each) do |example|
#       begin
#         GC.stress = true
#         example.run
#       ensure
#         GC.stress = false
#       end
#     end
#   end
# # This code causes a new profile file to be created for each test case which is excessive and hard to read
# #  c.around(:each) do |example|
# #    if ENV.key?("PROFILE")
# #      klass = example.metadata[:example_group][:example_group][:description_args][0].to_s.gsub(/::/,'')
# #      method = example.metadata[:description_args][0].to_s.gsub!(/ /,'_')
# #      RubyProf.start
# #      100.times do
# #        example.run
# #      end
# #      result = RubyProf.stop
# #      result.eliminate_methods!([/RSpec/, /BasicObject/])
# #      printer = RubyProf::GraphHtmlPrinter.new(result)
# #      dir = "./profile/#{klass}"
# #      FileUtils.mkdir_p(dir)
# #      printer.print(File.open("#{dir}/#{method}.html", 'w+'), :min_percent => 2)
# #    else
# #      example.run
# #    end
# #  end
# end
