Scene Builder README Version 0.1
================================

Overview
--------

Scene Builder allows you to create scenes for use in the Zippy Simulator with a simple create and publish workflow


How to Use
----------

- Create a new Unity project
- Import the Scene Builder package
- Create a new scene root (Zippy > Create Scene Root)
- Add all models under the Scene Root
- Make sure you include a light in the scene root. Either move the default one or make a new one
- Save the scene
- Publish the scene (Zippy > Publish Scene)
- Move the generated scene file (.zss) to somewhere sensible such as a folder on a shared server or AWS
- Do not check in the generated scene files

Compatibility
---------
- You can use any platform to create a scene (Windows/Mac/Linux)
- Use Unity 5.6.1 to maintain compatibility with the simulator
- The Linux editor is buggy, and the Zippy menu items may be disabled for no reason (along with other Unity menu items). If this happens, restart Unity or don't use the Linux editor.


How to use scenes in the simulator
----------------------------------

Scenes can be stored locally or on the web. A url should be either a http://, https:// or file://

Via settings file
- Edit the sceneUrl to be the url to the file
- Run the simulator as normal with the -settings SETTINGS_FILE command line option

Via the scene option
- Use -scene SCENE_URL command line option
- This overrides anthing in the settings file
- the sceneURL option in the settings file can be empty


Advanced Scene Creation
-----------------------

To make scenes look good and performant takes effort. Here are some quick tips to keep things working well
- Once the scene is setup, mark all static items as static (select them and then click the static tick box in the top right of the inspector)
- Make sure there is at least 1 light in the scene root to light the scene
- Bake the lighting. 
	- Make sure static things are set as static (see above), 
	- then go to Window > Lighting > settings
	- (Optional) Change the Lightmap Parameters to Default-LowResolution or Default-VeryLowResolution. You can increase these later to get better results once you know how well everything is working
	- Untick Auto Generate
	- Click 'Generate Lighting' to bake the lighting. Depending on how complex your scene is, you may need to go and make a coffee, have lunch, or come basck tomorrow.
	- Everytime you change the scene, you will need to bake the lighting again, so make it the last thing you do.


Caveats
-------
- You cannot include scripts in the scenes that are not in the simulator. If you need to script something, make sure the script is added to the simulator too.
- Bundles and loading them is very unity version specific. 
	- Usually bundles created by a major version are compatible with all other versions of the same major version. eg. a bundle made with 5.1 woulf be compatible with the app made with 5.6. But sometimes it is not the case and Unity are bad at letting people know.
	- Point releases should remain compatible
	- When the version of Unity changes to one that is incompatible, all the scenes will need to be rebuilt to work with the simulator again.
	- So take changing version seriously, as it may break all the existing scenes.

