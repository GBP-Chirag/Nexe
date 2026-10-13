import click

@click.group()
def cli():
    """Nexe CLI - A command-line interface for managing Nexe projects."""
    pass

@cli.command()
@click.argument('target')
@click.option('--profile', default='debug', help='Build profile (debug/release)')
def run(target, profile):
    """Compile and execute a specific target."""
    click.echo(f"Running target {target} in {profile} mode...")

if __name__ == '__main__':
    cli()
