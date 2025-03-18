#!/bin/bash
#
# Script to run Docker commands for NVMe-oF Benchmarking Suite
#

set -e

# Determine script directory and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"
DOCKER_DIR="$PROJECT_ROOT/docker"

# Parse command line arguments
ACTION=""
ENV="dev"
ARGS=""

print_usage() {
    echo "Usage: $0 [OPTIONS] ACTION"
    echo ""
    echo "Actions:"
    echo "  build       Build Docker image"
    echo "  run         Run Docker container (starts a new container)"
    echo "  exec        Execute command in running container"
    echo "  stop        Stop running container"
    echo "  bash        Start bash shell in container"
    echo "  benchmark   Run benchmark in container"
    echo ""
    echo "Options:"
    echo "  -e, --env ENV    Environment: 'dev' or 'prod' (default: 'dev')"
    echo "  -a, --args ARGS  Additional arguments to pass to Docker"
    echo "  -h, --help       Display this help message"
    echo ""
    echo "Examples:"
    echo "  $0 build                 # Build development image"
    echo "  $0 --env prod build      # Build production image"
    echo "  $0 run                   # Run development container"
    echo "  $0 bash                  # Start bash shell in development container"
    echo "  $0 --env prod benchmark  # Run benchmark in production container"
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        -e|--env)
            ENV="$2"
            shift 2
            ;;
        -a|--args)
            ARGS="$2"
            shift 2
            ;;
        -h|--help)
            print_usage
            exit 0
            ;;
        build|run|exec|stop|bash|benchmark)
            ACTION="$1"
            shift
            ;;
        *)
            echo "Error: Unknown option or action $1"
            print_usage
            exit 1
            ;;
    esac
done

if [ -z "$ACTION" ]; then
    echo "Error: No action specified"
    print_usage
    exit 1
fi

# Set container and service names based on environment
if [ "$ENV" = "dev" ]; then
    SERVICE="nvmeof-development"
    CONTAINER="nvmeof-development"
    DOCKERFILE="Dockerfile.dev"
elif [ "$ENV" = "prod" ]; then
    SERVICE="nvmeof-benchmarking"
    CONTAINER="nvmeof-benchmarking"
    DOCKERFILE="Dockerfile"
else
    echo "Error: Unknown environment '$ENV'. Use 'dev' or 'prod'."
    exit 1
fi

# Execute the requested action
case "$ACTION" in
    build)
        echo "Building $ENV Docker image..."
        cd "$DOCKER_DIR"
        docker-compose build $SERVICE $ARGS
        ;;
    run)
        echo "Running $ENV Docker container..."
        cd "$DOCKER_DIR"
        docker-compose up -d $SERVICE $ARGS
        ;;
    exec)
        echo "Executing command in $ENV Docker container..."
        docker exec $CONTAINER $ARGS
        ;;
    stop)
        echo "Stopping $ENV Docker container..."
        cd "$DOCKER_DIR"
        docker-compose stop $SERVICE
        ;;
    bash)
        echo "Starting bash shell in $ENV Docker container..."
        docker exec -it $CONTAINER /bin/bash
        ;;
    benchmark)
        echo "Running benchmark in $ENV Docker container..."
        cd "$DOCKER_DIR"
        if [ "$ENV" = "prod" ]; then
            docker-compose up nvmeof-benchmarking $ARGS
        else
            docker exec -it $CONTAINER /app/build/bin/nvmeof_benchmarking $ARGS
        fi
        ;;
    *)
        echo "Error: Unknown action '$ACTION'"
        print_usage
        exit 1
        ;;
esac

echo "Done."
