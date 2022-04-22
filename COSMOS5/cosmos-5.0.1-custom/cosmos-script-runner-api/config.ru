# This file is used by Rack-based servers to start the application.

require_relative 'config/environment'
require 'prometheus/middleware/collector'
require 'prometheus/middleware/exporter'

use Prometheus::Middleware::Collector
use Prometheus::Middleware::Exporter, {:path => '/script-api/metrics'}

run Rails.application
