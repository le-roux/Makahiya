from setuptools import setup

requires = [
    'pyramid',
    'sqlalchemy',
    'zope.sqlalchemy',
    'deform'
]

setup(name='makahiya',
      version='0.0',
      description='Server of Makahiya (ROSE 2017 project made at Telecom ParisTech)',
      url='makahiya.rfc1149.net',
      install_requires=requires,
      entry_points="""\
      [paste.app_factory]
      main = makahiya:main
      [console_scripts]
      initialize_db = makahiya.initialize_db:main
      initialize_test_db = makahiya.initialize_test_db:main
      """,
)
