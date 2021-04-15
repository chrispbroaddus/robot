import google.protobuf.text_format as tf
import google.protobuf.json_format as jf


class BasicConfigurationItem(object):
    """
    Common base class for all CI items. Captures common book-keeping details.
    """
    def __init__(self, name, file, is_global, logger):
        """
        :param name: name of this configuration item 
        :param file: file name (without any directory information) 
        :param is_global: true if this is a global CI, otherwise false for device-specific CIs
        """
        self.__name = name
        self.__is_global = is_global
        self.__file = file
        self.logger = logger

    def is_global(self):
        """
        Final.
        :return: true if this is a global configuration item, false otherwise 
        """
        return self.__is_global

    def name(self):
        """
        Final.
        :return: name of this configuration item 
        """
        return self.__name

    def validate(self, file_path):
        """
        Validate the file. It is expected that subclasses will override this method to perform any necessary sanity 
        checks.
        :param file_path: Location on disk of the CI 
        :return: 
        """
        pass

    def file(self):
        """
        Final.
        :return: the file name associated with this configuration item 
        """
        return self.__file


class ProtobufConfigurationItem(BasicConfigurationItem):
    """
    ConfigurationItem that uses protobuf to actually attempt to load configuration files during validation. If the 
    load succeeds (that is, the file is at least syntactically valid), declare success. 
    """
    def __init__(self, name, file, is_global, logger, protobuf_instance):
        super(ProtobufConfigurationItem, self).__init__(name, file, is_global, logger)
        self.__protobuf_instance = protobuf_instance

    def validate(self, file_path):
        self.logger.info('Checking text protobuf configuration file [{}]'.format(str(file_path)))
        with open(str(file_path), 'rt') as f:
            self.__protobuf_instance.Clear()
            tf.Merge(f.read(), self.__protobuf_instance)
            self.validate_contents(self.__protobuf_instance)

    def validate_contents(self, contents):
        """
        Subclasses can override this method to validate the actual contents of the loaded file
        :param contents: 
        :return: 
        """
        pass

class ProtobufJsonConfigurationItem(BasicConfigurationItem):
    """
    ConfigurationItem that uses protobuf to actually attempt to load configuration files stored in Json Format during validation.
    If the load succeeds (that is, the file is at least syntactically valid), declare success.
    """
    def __init__(self, name, file, is_global, logger, protobuf_instance):
        super(ProtobufJsonConfigurationItem, self).__init__(name, file, is_global, logger)
        self.__protobuf_instance = protobuf_instance

    def validate(self, file_path):
        self.logger.info('Checking json protobuf configuration file [{}]'.format(str(file_path)))
        with open(str(file_path), 'rt') as f:
            self.__protobuf_instance.Clear()
            jf.Parse(f.read(), self.__protobuf_instance)
            self.validate_contents(self.__protobuf_instance)

    def validate_contents(self, contents):
        """
        Subclasses can override this method to validate the actual contents of the loaded file
        :param contents:
        :return:
        """
        pass
