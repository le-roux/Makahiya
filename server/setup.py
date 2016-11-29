from setuptools import setup

requires = [
    'pyramid',
    'sqlalchemy',
    'zope.sqlalchemy'
]

setup(name='makahiya',
      install_requires=requires,
      entry_points="""\
      [paste.app_factory]
      main = makahiya:main
      [console_scripts]
      initialize_db = makahiya.initialize_db:main
      """,
)
