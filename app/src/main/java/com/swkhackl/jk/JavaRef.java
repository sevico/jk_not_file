package com.swkhackl.jk;

import java.lang.reflect.Field;
import java.lang.reflect.Method;


/**
 * Created by F8LEFT on 2017/3/23.
 * used java reflex to get/set method, field
 */

public class JavaRef {
    public static Method getMethod(String class_name, String method_name, Class[] pareType) {
        try {
            Class obj_class = Class.forName(class_name);
            Method method = obj_class.getDeclaredMethod(method_name,pareType);
            method.setAccessible(true);
            return method;
        } catch (Exception e) {
            Global.LogE("JavaRef", "getMethod " + e.toString());
        }
        return null;
    }

    public static Method getMethod(Class clazz, String method_name, Class[] pareType) {
        try {
            Method method = clazz.getDeclaredMethod(method_name,pareType);
            method.setAccessible(true);
            return method;
        } catch (Exception e) {
            Global.LogE("JavaRef", "getMethod " + e.toString());
        }
        return null;
    }


    public static Method getMethodByFirstName(Class clazz, String method_name) {
        try {
            Method[] methods = clazz.getDeclaredMethods();
            for(Method method:methods) {
                if (method.getName().equals(method_name)) {
                    method.setAccessible(true);
                    return method;
                }
            }
        } catch (Exception e) {
            Global.LogE("JavaRef", "getMethodByName " + e.toString());
        }
        return null;
    }

    public static Object invokeMethod(String class_name, String method_name, Class[] pareTyple, Object obj , Object[] pareVaules){
        try {
            Method method = getMethod(class_name, method_name, pareTyple);
            method.setAccessible(true);
            return method.invoke(obj, pareVaules);
        } catch (Exception e) {
            e.printStackTrace();
            Global.LogE("JavaRef", "invokeMethod " + e.toString());
        }
        return null;
    }

    public static Object invokeMethod(Class clazz, String method_name, Class[] pareTpye, Object obj, Object[] pareVaules) {
        try {
            Method method = getMethod(clazz, method_name, pareTpye);
            method.setAccessible(true);
            return method.invoke(obj, pareVaules);
        } catch (Exception e) {
            e.printStackTrace();
            Global.LogE("JavaRef", "invokeMethod " + e.toString());
        }
        return null;
    }

    public static Object invokeStaticMethod(String class_name, String method_name, Class[] pareTyple, Object[] pareVaules){
        return invokeMethod(class_name, method_name, pareTyple, null, pareVaules);
    }

    public static Object invokeStaticMethod(Class clazz, String method_name, Class[] pareTyple, Object[] pareVaules){
        return invokeMethod(clazz, method_name, pareTyple, null, pareVaules);
    }

    public static Object getFieldObject(String class_name, String filedName, Object obj){
        try {
            Class obj_class = Class.forName(class_name);
            Field field = obj_class.getDeclaredField(filedName);
            field.setAccessible(true);
            return field.get(obj);
        } catch (Exception e) {
            Global.LogE("JavaRef", "getFieldObject " + e.toString());
        }
        return null;
    }

    public static Class getClass(String class_name) {
        try {
            return Class.forName(class_name);
        } catch (Exception e) {
            Global.LogE("JavaRef", "getClass " + e.toString());
        }
        return null;
    }

    public static Object getFieldObject(Class clazz, String fieldName, Object obj) {
        try {
            Field field = clazz.getDeclaredField(fieldName);
            field.setAccessible(true);
            return field.get(obj);
        } catch (Exception e) {
            Global.LogE("JavaRef", "getFieldObject " + e.toString());
        }
        return null;
    }

    public static Object getStaticFieldObject(String class_name, String fieldName){
        return getFieldObject(class_name, fieldName, null);
    }

    public static Object getStaticFieldObject(Class clazz, String fieldName){
        return getFieldObject(clazz, fieldName, null);
    }

    // match first type
    public static Object getFieldObjectByTypeName(Class clazz,  String type, Object object) {
        try {
            Field[] fs = clazz.getDeclaredFields();

            for(Field f: fs) {
                String fieldType = f.getType().getName();
                if (fieldType.equals(type)) {
                    f.setAccessible(true);
                    return f.get(object);
                }
            }

        } catch (Exception e) {
            Global.LogE("JavaRef", "getFieldObjectByTypeName " + e.toString());
        }
        return null;
    }

    // match first type
    public static Field getFieldByTypeName(Class clazz,  String type) {
        try {
            Field[] fs = clazz.getDeclaredFields();

            for(Field f: fs) {
                String fieldType = f.getType().getName();
                if (fieldType.equals(type)) {
                    f.setAccessible(true);
                    return f;
                }
            }

        } catch (Exception e) {
            Global.LogE("JavaRef", "getFieldByTypeName " + e.toString());
        }
        return null;
    }

    public static Field getField(Class clazz, String name) {
        try {
            Field f = clazz.getDeclaredField(name);
            f.setAccessible(true);
            return f;
        } catch (Exception e) {
            Global.LogE("JavaRef", "getField " + e.toString());
        }
        return null;
    }

    public static void setFieldObject(String classname, String filedName, Object obj, Object filedVaule){
        try {
            Class obj_class = Class.forName(classname);
            Field field = obj_class.getDeclaredField(filedName);
            field.setAccessible(true);
            field.set(obj, filedVaule);
        } catch (Exception e) {
            Global.LogE("JavaRef", "setFieldObject " + e.toString());
        }
    }

    public static void setFieldObject(Class clazz, String filedName, Object obj, Object filedVaule){
        try {
            Field field = clazz.getDeclaredField(filedName);
            field.setAccessible(true);
            field.set(obj, filedVaule);
        } catch (Exception e) {
            Global.LogE("JavaRef", "setFieldObject " + e.toString());
        }
    }

    public static void setStaticFieldObject(String classname, String fieldName, Object fieldValue){
        setFieldObject(classname, fieldName, null, fieldValue);
    }

    public static void setStaticFieldObject(Class clazz, String fieldName, Object fieldValue){
        setFieldObject(clazz, fieldName, null, fieldValue);
    }

    public static void logClass(Class clazz) {
        try {
            Global.LogI("JavaRef", clazz.toString());

            Field[] fs = clazz.getDeclaredFields();

            for(Field f: fs) {
                Global.LogI("JavaRef", "logClass field :" + f.toString());
            }
            Method[] ms = clazz.getDeclaredMethods();

            for(Method m: ms) {
                Global.LogI("JavaRef", "logClass method :" + m.toString());
            }
        } catch (Exception e) {
            Global.LogE("JavaRef", "logClass " + e.toString());
        }
    }

    public static void logObject(Object o) {
        try {
            Class clazz = o.getClass();
            Global.LogI("JavaRef", clazz.toString());

            Field[] fs = clazz.getDeclaredFields();

            for(Field f: fs) {
                boolean isAcc = f.isAccessible();
                f.setAccessible(true);
                Global.LogI("JavaRef", "logObject field :" + f);
                Global.LogI("JavaRef", "Value: " + f.get(o));
                f.setAccessible(isAcc);
            }
//            Method[] ms = clazz.getDeclaredMethods();
//
//            for(Method m: ms) {
//                Global.LogD("JavaRef", "logObject method :" + m.toString());
//            }
        } catch (Exception e) {
            Global.LogE("JavaRef", "logObject " + e.toString());
        }
    }
}
