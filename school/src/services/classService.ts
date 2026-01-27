import { api } from './api';
import type { Class } from '@/types';

export const classService = {
  async getAll(): Promise<Class[]> {
    return api.get<Class[]>('/classes');
  },

  async getById(id: string): Promise<Class> {
    return api.get<Class>(`/classes/${id}`);
  },

  async create(classData: Omit<Class, 'id' | 'studentCount'>): Promise<Class> {
    return api.post<Class>('/classes', classData);
  },

  async update(id: string, classData: Partial<Class>): Promise<Class> {
    return api.patch<Class>(`/classes/${id}`, classData);
  },

  async delete(id: string): Promise<void> {
    return api.delete(`/classes/${id}`);
  },
};
